#include "syscall.h"

#include "interrupts.h"
#include "log.h"
#include "util.h"
#include "tasks.h"
#include "ramdisk.h"
#include "userheap.h"
#include "time.h"

#define NUM_SYSCALLS 256
void* syscall_handlers[NUM_SYSCALLS];

static void handle_syscall_interrupt(TrapFrame* frame);

int syscall_get_task_id() {
    return current_task->id;
}

void syscall_exit() {
    kill_task(current_task->id);
}

void syscall_print(const char* string) {
    kernel_log("task %u: %s", current_task->id, string);
}

void syscall_exec(const char* path) {
    // kernel_log("task %u exec: %s", current_task->id, path);
    create_user_task(path);
}

u32 syscall_open_file(const char* path) {
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (current_task->open_files[i].obj.fs == 0) {
            fd = i;
            break;
        }
    }
    assert_msg(fd != -1, "too many opened files!");
    kernel_log("fd=%d %s", fd, path);

    FRESULT res = f_open(&current_task->open_files[fd], path, FA_READ);
    assert_msg(res == FR_OK, "f_open");

    kernel_log("opened file %s with fd=%d", path, fd);
    return fd;
}

u32 syscall_read_file(u32 fd, u8* buf, u32 num_bytes) {
    assert_msg(fd < MAX_OPEN_FILES, "");
    assert_msg(current_task->open_files[fd].obj.fs != 0, "");
    UINT br;
    FRESULT res = f_read(&current_task->open_files[fd], buf, num_bytes, &br);
    assert_msg(res == FR_OK, "");
    return br;
}

void syscall_close_file(u32 fd) {
    assert_msg(fd < MAX_OPEN_FILES, "");

    FRESULT res = f_close(&current_task->open_files[fd]);
    assert_msg(res == FR_OK, "");
}

u32 syscall_get_file_size(u32 fd) {
    assert_msg(fd < MAX_OPEN_FILES, "");

    return f_size(&current_task->open_files[fd]);
}

u32 syscall_get_heap_start() {
    return current_task->heap_start;
}

u32 syscall_get_heap_end() {
    return current_task->heap_end;
}

void syscall_set_heap_end(u32 heap_end) {
    set_user_heap_end(current_task, heap_end);
}

int syscall_open_dir(const char* path) {
    return f_opendir(&current_task->open_dir, path) == FR_OK;
}

int syscall_close_dir() {
    return f_closedir(&current_task->open_dir) == FR_OK;
}

typedef struct {
    uint32_t attributes;
    char name[256];
} OSFileInfo;
#define OS_FILE_INFO_IS_DIR 16

int syscall_next_file_in_dir(OSFileInfo* info_out) {
    FILINFO info;
    f_readdir(&current_task->open_dir, &info);
    
    if (info.fname[0] == '\0')
        return 0;

    info_out->attributes = 0;
    if (info.fattrib & AM_DIR) {
        info_out->attributes |= OS_FILE_INFO_IS_DIR;
    }

    strncpy(info_out->name, info.fname, sizeof(info.fname));
    return 1;
}

void syscall_set_timer_interval(int timer_id, int interval_ms) {
    assert_msg(timer_id < MAX_TIMERS, "too many timers!");
    Timer* timer = &current_task->timers[timer_id];
    timer->interval = interval_ms;
    timer->active = interval_ms != 0;
    timer->next_fire = get_system_time_millis() + interval_ms;
}

void init_syscalls() {
    memset(syscall_handlers, 0, sizeof(syscall_handlers));

    register_syscall(SYSCALL_GET_TASK_ID, syscall_get_task_id);
    register_syscall(SYSCALL_EXIT, syscall_exit);
    register_syscall(SYSCALL_PRINT, syscall_print);
    register_syscall(SYSCALL_EXEC, syscall_exec);
    register_syscall(SYSCALL_OPEN_FILE, syscall_open_file);
    register_syscall(SYSCALL_CLOSE_FILE, syscall_close_file);
    register_syscall(SYSCALL_READ_FILE, syscall_read_file);
    register_syscall(SYSCALL_GET_FILE_SIZE, syscall_get_file_size);
    register_syscall(SYSCALL_GET_HEAP_START, syscall_get_heap_start);
    register_syscall(SYSCALL_GET_HEAP_END, syscall_get_heap_end);
    register_syscall(SYSCALL_SET_HEAP_END, syscall_set_heap_end);
    register_syscall(SYSCALL_OPEN_DIR, syscall_open_dir);
    register_syscall(SYSCALL_CLOSE_DIR, syscall_close_dir);
    register_syscall(SYSCALL_NEXT_FILE_IN_DIR, syscall_next_file_in_dir);
    register_syscall(SYSCALL_SET_TIMER_INTERVAL, syscall_set_timer_interval);

    register_isr(0x80, handle_syscall_interrupt);
}

void register_syscall(u32 vector, void* func) {
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
        : "=a" (ret) : "r" (frame->edi), "r" (frame->esi), "r" (frame->edx), "r" (frame->ecx), "r" (frame->ebx), "r" (handler)
    );
    frame->eax = ret;
}
