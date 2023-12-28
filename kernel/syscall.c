#include "syscall.h"

#include "interrupts.h"
#include "log.h"
#include "util.h"
#include "tasks.h"
#include "userheap.h"
#include "time.h"
#include "sharedmem.h"
#include "physalloc.h"
#include "memory.h"
#include "disk.h"

#define NUM_SYSCALLS 1024
void* syscall_handlers[NUM_SYSCALLS];

static void handle_syscall_interrupt(TrapFrame* frame);

static int syscall_get_task_id() {
    return current_task->id;
}

static void syscall_exit() {
    kill_task(current_task->id);
}

static void syscall_print(const char* string) {
    // kernel_log("task %u: %s", current_task->id, string);
    kernel_log("%s", string);
}

// "temporary" solution.
static void syscall_print_char(char c) {
    kernel_log_char(c);
}

// FIXME: this looks safe right?
static void syscall_exec(const char* path, const char* argv[]) {
    // kernel_log("task %u exec: %s", current_task->id, path);
    create_user_task(path, argv);
}

// FIXME: specify mode?
static u32 syscall_open_file(const char* path) {
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (current_task->open_files[i].cluster == 0) {
            fd = i;
            break;
        }
    }
    assert_msg(fd != -1, "too many opened files!");
    // kernel_log("--> syscall_open_file   fd=%d %s", fd, path);

    if (!fat32_find_file(&ramdisk.volume, path, &current_task->open_files[fd])) {
        assert(false);
    }

    kernel_log("opened file %s with fd=%d size=%u", path, fd, current_task->open_files[fd].size);
    return fd + 1000;
}

static u32 syscall_read_file(u32 fd, u8* buf, u32 num_bytes) {
    fd -= 1000;
    assert(fd < MAX_OPEN_FILES);
    assert(current_task->open_files[fd].cluster != 0);
    assert(buf + num_bytes < KERNEL_START);
    if (num_bytes == 0)
        return 0;

    FAT32_File* file = &current_task->open_files[fd];
    assert(file->cluster != 0);

    // printf("syscall_read_file num_bytes=%u offset=%u\n", num_bytes, file->offset);
    fat32_read_file(&ramdisk.volume, file, buf, num_bytes, file->offset);
    file->offset += num_bytes;

    return num_bytes;
}

static u32 syscall_write_file(u32 fd, u8* buf, u32 num_bytes) {
    assert(false);
    // TODO: 
    // fd -= 1000;
    // assert(fd < MAX_OPEN_FILES);
    // assert(current_task->open_files[fd].obj.fs != 0);
    // if (num_bytes == 0)
    //     return 0;
    
    // UINT br;
    // FRESULT res = f_write(&current_task->open_files[fd], buf, num_bytes, &br);
    // assert(res == FR_OK);
    return 0;
}

static void syscall_close_file(u32 fd) {
    fd -= 1000;
    assert(fd < MAX_OPEN_FILES);
    FAT32_File* file = &current_task->open_files[fd];
    assert(file->cluster != 0);
    file->cluster = 0;
}

static u32 syscall_get_file_size(u32 fd) {
    fd -= 1000;
    assert(fd < MAX_OPEN_FILES);
    FAT32_File* file = &current_task->open_files[fd];
    assert(file->cluster != 0);

    return file->size;
}

static u32 syscall_get_file_offset(u32 fd) {
    fd -= 1000;
    assert(fd < MAX_OPEN_FILES);
    FAT32_File* file = &current_task->open_files[fd];
    assert(file->cluster != 0);

    return file->offset;
}

static u32 syscall_set_file_offset(u32 fd, u32 offset) {
    fd -= 1000;
    assert(fd < MAX_OPEN_FILES);
    FAT32_File* file = &current_task->open_files[fd];
    assert(file->cluster != 0);
    // kernel_log("syscall_set_file_offset offset=%u", offset);
    assert(offset <= file->size);

    file->offset = offset;
    return 0;
}

static u32 syscall_get_heap_start() {
    return current_task->heap_start;
}

static u32 syscall_get_heap_end() {
    return current_task->heap_end;
}

static void syscall_set_heap_end(u32 heap_end) {
    set_user_heap_end(current_task, heap_end);
}

static int syscall_open_dir(const char* path) {
    FAT32_File dir_file;
    if (!fat32_find_file(&ramdisk.volume, path, &dir_file)) {
        assert(0);
    }

    fat32_list_dir(&ramdisk.volume, &dir_file, &current_task->dir_list);
    return 0;
}

static int syscall_close_dir() {
    current_task->dir_list.cluster = 0;
    return 0;
}

typedef struct {
    uint32_t attributes;
    char name[256];
} OSFileInfo;
#define OS_FILE_INFO_IS_DIR 16

static int syscall_next_file_in_dir(OSFileInfo* info_out) {
    assert(current_task->dir_list.cluster != 0);

    FAT32_File file;
    if (!fat32_next_dir_entry(&ramdisk.volume, &current_task->dir_list, &file, info_out->name)) {
        return 0;
    }

    if (file.attrib & FAT32_IS_DIR) {
        info_out->attributes |= OS_FILE_INFO_IS_DIR;
    }

    return 1;
}

static void syscall_set_timer_interval(int timer_id, int interval_ms) {
    assert_msg(timer_id < MAX_TIMERS, "too many timers!");
    Timer* timer = &current_task->timers[timer_id];
    timer->interval = interval_ms;
    timer->active = interval_ms != 0;
    timer->next_fire = get_system_time_millis() + interval_ms;
}

static void* syscall_sharedmem_map(s32 id) {
    return sharedmem_map(id, current_task->id);
}

static s32 syscall_shmem_create(u32 size) {
    return sharedmem_create(size, current_task->id);
}

static s32 syscall_shmem_destroy(s32 id) {
    if (id < 0 || id >= MAX_SHARED_MEMORY_OBJS)
        assert(0);
    
    u32 task_id = current_task->id;
    // kernel_log("id = %d task_id = %d owner_task_id = %u", id, task_id, shmem[id].owner_task_id);
    if (shmem[id].owner_task_id != task_id)
        assert(0);
    
    sharedmem_destroy(id);
    return 0;
}

static bool syscall_sharedmem_exists(s32 id) {
    return sharedmem_exists(id);
}

static void syscall_sharedmem_unmap(s32 id) {
    sharedmem_unmap(id, current_task->id);
}

static void syscall_debug() {
    kernel_log("-- syscall debug BEGIN");
    kernel_log("our pdir=%x", mem_get_current_page_directory());

    u32 phys = pmm_alloc_pageframe();

    for (u32 i = 0; i < 1024 * 1024; i++) {
        u32 valid = mem_is_valid_vaddr(i * 0x1000);
        if (valid)
            kernel_log("%x is valid!", i * 0x1000);
    }

    kernel_log("-- syscall debug FINISH");
}

// FIXME: u64
static u32 syscall_get_system_time() {
    return (u32) get_system_time_millis();
}

typedef struct {
    uint32_t id;
    uint32_t state;
} OSTaskInfo;

os_errorcode syscall_get_task_info(OSTaskInfo* list, uint32_t list_max_size, uint32_t* num_tasks) {
    if (list > KERNEL_START)
        return 1;
    
    int index = 0;
    for (int i = 0; i < MAX_TASKS; i++) {
        Task* task = &tasks[i];
        if (task->state == TASK_STATE_DEAD)
            continue;
        
        OSTaskInfo* info = list + index;
        info->id = task->id;
        info->state = task->state;
    
        index++;
        if (index >= list_max_size)
            break;
    }

    *num_tasks = index;

    return 0;
}

void init_syscalls() {
    memset(syscall_handlers, 0, sizeof(syscall_handlers));

    register_syscall(SYSCALL_GET_TASK_ID, syscall_get_task_id);
    register_syscall(SYSCALL_EXIT, syscall_exit);
    register_syscall(SYSCALL_PRINT, syscall_print);
    register_syscall(SYSCALL_PRINT_CHAR, syscall_print_char);
    register_syscall(SYSCALL_EXEC, syscall_exec);
    register_syscall(SYSCALL_GET_HEAP_START, syscall_get_heap_start);
    register_syscall(SYSCALL_GET_HEAP_END, syscall_get_heap_end);
    register_syscall(SYSCALL_SET_HEAP_END, syscall_set_heap_end);
    register_syscall(SYSCALL_SET_TIMER_INTERVAL, syscall_set_timer_interval);
    register_syscall(SYSCALL_GET_SYSTEM_TIME, syscall_get_system_time);

    // file io
    register_syscall(SYSCALL_OPEN_FILE, syscall_open_file);
    register_syscall(SYSCALL_CLOSE_FILE, syscall_close_file);
    register_syscall(SYSCALL_READ_FILE, syscall_read_file);
    register_syscall(SYSCALL_WRITE_FILE, syscall_write_file);
    register_syscall(SYSCALL_GET_FILE_SIZE, syscall_get_file_size);
    register_syscall(SYSCALL_GET_FILE_OFFSET, syscall_get_file_offset);
    register_syscall(SYSCALL_SET_FILE_OFFSET, syscall_set_file_offset);

    register_syscall(SYSCALL_OPEN_DIR, syscall_open_dir);
    register_syscall(SYSCALL_CLOSE_DIR, syscall_close_dir);
    register_syscall(SYSCALL_NEXT_FILE_IN_DIR, syscall_next_file_in_dir);

    // shared mem
    register_syscall(SYSCALL_SHMEM_CREATE, syscall_shmem_create);
    register_syscall(SYSCALL_SHMEM_DESTROY, syscall_shmem_destroy);
    register_syscall(SYSCALL_SHMEM_EXISTS, syscall_sharedmem_exists);
    register_syscall(SYSCALL_SHMEM_MAP, syscall_sharedmem_map);
    register_syscall(SYSCALL_SHMEM_UNMAP, syscall_sharedmem_unmap);

    register_syscall(SYSCALL_DEBUG, syscall_debug);
    register_syscall(SYSCALL_GET_TASK_INFO, syscall_get_task_info);

    register_isr(0x80, handle_syscall_interrupt);
}

void register_syscall(u32 vector, void* func) {
    assert(vector < NUM_SYSCALLS);
    assert(syscall_handlers[vector] == 0);
    syscall_handlers[vector] = func;
}

static void handle_syscall_interrupt(TrapFrame* frame) {
    // kernel_log("syscall %u", frame->eax);

    u32 vector = frame->eax;
    void* handler = syscall_handlers[vector];

    assert_msg(handler, "invalid syscall");

    int ret;
    asm volatile(
        "push %1 \n"
        "push %2 \n"
        "push %3 \n"
        "push %4 \n"
        "push %5 \n"
        "call *%6 \n"
        "pop %%ebx \n"
        "pop %%ebx \n"
        "pop %%ebx \n"
        "pop %%ebx \n"
        "pop %%ebx \n"
        : "=a" (ret)
        : "r" (frame->edi), "r" (frame->esi), "r" (frame->edx), "r" (frame->ecx), "r" (frame->ebx), "r" (handler)
    );
    frame->eax = ret;
}
