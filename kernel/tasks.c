#include "tasks.h"

#include "interrupts.h"
#include "gdt.h"
#include "util.h"
#include "memory.h"
#include "log.h"
#include "kmalloc.h"
#include "elf.h"
#include "events.h"
#include "userheap.h"
#include "windows.h"
#include "gui.h"
#include "kernel.h"
#include "disk.h"

#define KERNEL_STACK_SIZE (0x1000 - 16)

Task tasks[MAX_TASKS];
int num_tasks;
Task* current_task;
static int current_task_index;

extern void isr_exit();
extern void switch_context(Task* old, Task* new);

static Task* create_task(u32 index, u32 eip, bool kernel_task, u32* pagedir);
static u32 get_task_index(int task_id);
static u32 find_available_task_slot();
static u32 setup_task_init_data(const char* path, const char* argv[], char* buffer);

void setup_tasks() {
    memset(tasks, 0, sizeof(Task) * MAX_TASKS);

    num_tasks = 1;
    current_task = &tasks[0];
    current_task->id = 1000;
    current_task->state = TASK_STATE_IDLE;
    current_task->pagedir = mem_get_current_page_directory();
    current_task_index = 0;

    // task 0 represents the execution we're in right now
}

static char argv_buffer[0x1000];

s32 create_user_task(const char* path, const char* argv[]) {
    s32 task_id = -1;
    ELFObject elf = {0};
    u32* prev_pd = NULL;
    u32* pagedir = NULL;

    push_cli();

    kernel_log("starting task: %s", path);

    FAT32_File file;
    if (!fat32_find_file(&ramdisk.volume, path, &file)) {
        kernel_log("create_user_task: failed to open executable file '%s'", path);
        goto error;
    }

    elf.size = file.size;
    if (elf.size < 52) {
        kernel_log("create_user_task: elf file is too small. size=%u", elf.size);
        goto error;
    }

    elf.raw = kmalloc(elf.size);

    // UINT br;
    // res = f_read(&file, elf.raw, elf.size, &br);

    fat32_read_file(&ramdisk.volume, &file, elf.raw, elf.size, 0);
    // FIXME
    // if (res != FR_OK) {
    //     kernel_log("create_user_task: failed to read from executable file %s. error=%u", path, res);
    //     goto error;
    // }

    int index = find_available_task_slot();

    // before entering the new page dir, we need to copy argv
    // since it can still point to the old tasks memory space

    memset(argv_buffer, 0, 0x1000);
    int argv_buffer_size = setup_task_init_data(path, argv, argv_buffer);

    // set up memory space
    prev_pd = mem_get_current_page_directory();
    pagedir = mem_alloc_page_dir();
    mem_change_page_directory(pagedir);

    // load elf, requires some memory to be set up
    if (!parse_elf(&elf)) {
        kernel_log("create_user_task: failed to parse ELF binary");
        goto error;
    }
    
    if (!load_elf_executable(&elf)) {
        kernel_log("create_user_task: failed to load ELF into memory");
        goto error;
    }

    if (!elf.entry) {
        kernel_log("create_user_task: ELF has no entry");
        goto error;
    }
    
    // init the task
    Task* new_task = create_task(index, elf.entry, false, pagedir);
    task_id = new_task->id;
    init_events_for_task(new_task);
    init_user_heap(new_task);
    init_shared_libs_for_task(task_id, &elf);

    // allocate and map user stack
    for (int i = 0; i < USER_STACK_PAGES; i++) {
        mem_map_page(USER_STACK_BOTTOM - USER_STACK_PAGES * 0x1000 + i * 0x1000, pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    }

    // map and copy argv
    mem_map_page(TASK_INIT_DATA, pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER);
    memcpy(TASK_INIT_DATA, argv_buffer, 0x1000);
    
    // for (int i = 0; i < MAX_OPEN_FILES; i++) {
    //     f_close(&new_task->open_files[i]);
    // }

    goto cleanup;
error:
    if (pagedir != NULL)
        mem_free_page_dir(pagedir);
    if (new_task != NULL)
        kill_task(new_task->id);
cleanup:
    if (prev_pd != NULL)
        mem_change_page_directory(prev_pd);
    // f_close(&file);
    if (elf.raw != NULL)
        kfree(elf.raw);

    pop_cli();
    return task_id;
}

s32 create_kernel_task(void* func) {
    int index = find_available_task_slot();

    Task* task = create_task(index, (u32) func, true, initial_page_dir);

    return task->id;
}

void kill_task(u32 id) {
    push_cli();

    kernel_log("kill_task %u", id);
    if (id <= 1000) {
        kernel_log("task id must be over 1000");
        return;
    }

    int index = get_task_index(id);
    Task* task = &tasks[index];
    // kernel_log("killing task %u", index);

    if (task->state == TASK_STATE_DEAD) {
        kernel_log("task is already dead!");
        return;
    }

    if (!task->is_kernel_task) {
        destroy_events_for_task(task);
    }

    // FIXME: destroy user owned shmem objs
    for (int i = 0; i < MAX_SHARED_MEMORY_OBJS; i++) {
        if (shmem[i].owner_task_id == id) {
            sharedmem_destroy(i);
        }
    }

    destroy_all_windows_belonging_to(task->id);

    mem_free_page_dir(task->pagedir);
    kfree(task->kesp0 - KERNEL_STACK_SIZE);
    memset(task, 0, sizeof(Task));

    num_tasks--;

    pop_cli();
}

// set up initial state of task
static Task* create_task(u32 index, u32 eip, bool kernel_task, u32* pagedir) {
    assert(tasks[index].id == 0);

    Task* task = tasks + index;

    memset(&tasks[index], 0, sizeof(Task));
    
    // each task has its own kernel stack for syscalls.
    // setup initial kernel stack
    u32 kernel_stack = ((u32) kmalloc(KERNEL_STACK_SIZE)) + KERNEL_STACK_SIZE;
    u8* kesp = (u8*) (kernel_stack);
    
    kesp -= sizeof(TrapFrame);
    TrapFrame* trap = (TrapFrame*) kesp;
    memset(trap, 0, sizeof(TrapFrame));

    u32 code_selector = kernel_task ? GDT_KERNEL_CODE : (GDT_USER_CODE | DPL_USER);
    u32 data_selector = kernel_task ? GDT_KERNEL_DATA : (GDT_USER_DATA | DPL_USER);

    trap->cs = code_selector;
    trap->ds = data_selector;
    trap->es = data_selector;
    trap->fs = data_selector;
    trap->gs = data_selector;

    trap->usermode_ss = data_selector;
    trap->usermode_esp = USER_STACK_BOTTOM;

    trap->eflags = 0x200; // enable interrupts
    trap->eip = eip;

    kesp -= sizeof(TaskReturnContext);
    TaskReturnContext* context = (TaskReturnContext*) kesp;
    context->edi = 0;
    context->esi = 0;
    context->ebx = 0;
    context->ebp = 0;
    context->eip = (u32) isr_exit;

    task->kesp0 = kernel_stack;
    task->kesp = (u32) kesp;
    task->id = index + 1000;
    task->state = TASK_STATE_READY;
    task->pagedir = pagedir;
    task->is_kernel_task = kernel_task;
    task->shmem.vaddr_start = USER_SHARED_MEMORY;

    num_tasks++;

    return task;
}

static u32 get_task_index(int task_id) {
    assert(task_id >= 1000);
    return task_id - 1000;
}

static u32 find_available_task_slot() {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_STATE_DEAD) {
            return i;
        }
    }
    assert_msg(0, "no more task slots!");
}

Task* get_task(int id) {
    return &tasks[get_task_index(id)];
}

u32 setup_task_init_data(const char* path, const char* argv[], char* buffer) {
    // all of this is kinda silly.. but im in a silly mood >:)

    char* data = buffer;

    char* path_ptr = path;
    while (true) {
        *data = *path_ptr;
        if (*data == '\0')
            break;
        data++;
        path_ptr++;
    }

    data++;

    assert((data - buffer) < 0x1000);

    if (argv == NULL) {
        *data = '\0'; // last string is empty
        return data - buffer;
    }

    for (int i = 0; argv[i] != NULL; i++) {
        const char* arg = argv[i];

        char* arg_ptr = arg;
        while (true) {
            *data = *arg_ptr;
            data++;
            if (*arg_ptr == '\0')
                break;
            assert((data - buffer) < 0x1000);
            arg_ptr++;
        }
    }
    
    data++;
    assert((data - buffer) < 0x1000);
    *data = '\0'; // last string is empty
    return data - buffer;
}

// TODO: move to schedule.c?
static int choose_next_task() {
    if (graphics_enabled && should_gui_redraw()) { // gui task always gets priority
        return 1; // FIXME: gui task is not guaranteed to be at index 1
    }

    // naive scheduling: just cycle through all the tasks once
    int index = current_task_index;

    for (int i = 0; i < MAX_TASKS; i++) {
        index++;
        index %= MAX_TASKS;

        if (tasks[index].state == TASK_STATE_READY) {
            return index; // we found a ready task, could be the same as we were in before
        }
    }

    // couldnt find anyone else, go back to kernel task
    return 0;
}

void task_schedule() {
    int index = choose_next_task();

    Task* next = &tasks[index];
    Task* old = current_task;
    current_task = next;
    current_task_index = index;

    // update tss
    update_tss_esp0(next->kesp0);

    // switch context, may not return here
    switch_context(old, next);
}
