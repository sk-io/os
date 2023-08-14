#include "tasks.h"

#include "interrupts.h"
#include "gdt.h"
#include "util.h"
#include "memory.h"
#include "log.h"
#include "kmalloc.h"
#include "elf.h"
#include "ramdisk.h"
#include "events.h"
#include "userheap.h"

#define KERNEL_STACK_SIZE (0x1000 - 16)

Task tasks[MAX_TASKS];
int num_tasks;
Task* current_task;
static int current_task_index;

extern void isr_exit();
extern void switch_context(Task* old, Task* new);

static void create_task(u32 index, u32 eip, bool kernel_task, u32* pagedir);
static u32 get_task_index(int task_id);
static u32 find_available_task_slot();

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

s32 create_user_task(const char* path) {
    // todo: push/pop interrupt state
    u32 eflags = read_eflags();

    // kernel_log("create_user_task eflags=%u", eflags);
    if (eflags & FL_IF)
        disable_interrupts();

    FIL file;
    FRESULT res;
    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        kernel_log("create_user_task: failed to open executable file %s. error=%u", path, res);
        return -1;
    }
    
    u32 elf_size = f_size(&file);
    u8* elf = kmalloc(elf_size);
    UINT br;
    res = f_read(&file, elf, elf_size, &br);
    if (res != FR_OK) {
        f_close(&file);
        kfree(elf);
        kernel_log("create_user_task: failed to read from executable file %s. error=%u", path, res);
        return -1;
    }
    f_close(&file);

    int index = find_available_task_slot();

    // set up memory space
    u32* prev_pd = mem_get_current_page_directory();
    u32* pagedir = mem_alloc_page_dir();
    mem_change_page_directory(pagedir);

    for (int i = 0; i < USER_STACK_PAGES; i++) {
        mem_map_page(USER_STACK_BOTTOM - USER_STACK_PAGES * 0x1000 + i * 0x1000, pmm_alloc_pageframe(), PAGE_FLAG_OWNER | PAGE_FLAG_USER | PAGE_FLAG_WRITE);
    }

    // load elf, requires memory to be set up
    u32 entry = load_elf_segments(elf);
    kfree(elf);

    // init the task
    create_task(index, entry, false, pagedir);
    init_events_for_task(&tasks[index]);
    init_user_heap(&tasks[index]);

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        f_close(&tasks[index].open_files[i]);
    }
    mem_change_page_directory(prev_pd);

    num_tasks++;

    if (eflags & FL_IF)
        enable_interrupts();
    return tasks[index].id;
}

void create_kernel_task(void* func) {
    int index = find_available_task_slot();

    num_tasks++;
    create_task(index, (u32) func, true, initial_page_dir);
}

void kill_task(u32 id) {
    u32 eflags = read_eflags();
    if (eflags & FL_IF)
        disable_interrupts();

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

    mem_free_page_dir(task->pagedir);
    kfree(task->kesp0 - KERNEL_STACK_SIZE);
    memset(&tasks[index], 0, sizeof(Task));

    num_tasks--;

    if (eflags & FL_IF)
        enable_interrupts();
}

static int choose_next_task() {
    if (num_tasks == 1) {
        return 0;
    }

    // naive scheduling: just cycle through all the tasks
    int index = current_task_index;

    while (true) {
        index++;
        index %= MAX_TASKS;

        if (tasks[index].state == TASK_STATE_READY) {
            break; // we found one
        }
    }
    return index;
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

// set up initial state of task
static void create_task(u32 index, u32 eip, bool kernel_task, u32* pagedir) {
    assert(tasks[index].id == 0);

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

    tasks[index].kesp0 = kernel_stack;
    tasks[index].kesp = (u32) kesp;
    tasks[index].id = index + 1000;
    tasks[index].state = TASK_STATE_READY;
    tasks[index].pagedir = pagedir;
    tasks[index].is_kernel_task = kernel_task;
}

static u32 get_task_index(int task_id) {
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
