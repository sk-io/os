#include "os.h"

#include "../../../kernel/syscall_list.h"

int syscall_get_task_id() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_TASK_ID)
    );
    return ret;
}

void syscall_exit() {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_EXIT)
    );
}

void syscall_print(const char* str) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_PRINT), "b"(str)
    );
}

void syscall_print_char(char c) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_PRINT_CHAR), "b"(c)
    );
}

void syscall_exec(const char* path) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_EXEC), "b"(path)
    );
}

int syscall_get_event_buffer_shmem_id() {
    int shmem_id;
    asm volatile(
        "int $0x80"
        : "=a"(shmem_id) : "a"(SYSCALL_GET_EVENT_BUFFER_SHMEM_ID)
    );
    return shmem_id;
}

void syscall_wait_for_events() {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_WAIT_FOR_EVENT)
    );
}

void syscall_set_timer_interval(int timer_id, int interval_ms) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SET_TIMER_INTERVAL), "b"(timer_id), "c"(interval_ms)
    );
}

int syscall_create_window(int width, int height, unsigned int flags) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a"(SYSCALL_CREATE_WINDOW), "b"(width), "c"(height), "d"(flags)
    );
    return ret;
}

int syscall_destroy_window(int window_id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a"(SYSCALL_CREATE_WINDOW), "b"(window_id)
    );
    return ret;
}

int syscall_get_window_fb_shmem_id(int window_id) {
    int id;
    asm volatile(
        "int $0x80"
        : "=a" (id)
        : "a"(SYSCALL_GET_WINDOW_FB_SHMEM_ID), "b"(window_id)
    );
    return id;
}

int syscall_swap_window_buffers(int window_id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_WINDOW_SWAP_BUFFERS), "b"(window_id)
    );
    return ret;
}

int syscall_set_window_title(int window_id, const char* title) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_SET_WINDOW_TITLE), "b"(window_id), "c"(title)
    );
    return ret;
}

uint32_t syscall_get_heap_start() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_HEAP_START)
    );
    return ret;
}

uint32_t syscall_get_heap_end() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_HEAP_END)
    );
    return ret;
}

void syscall_set_heap_end(uint32_t heap_end) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SET_HEAP_END), "b"(heap_end)
    );
}

int syscall_open_file(const char* path) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_OPEN_FILE), "b"(path)
    );
    return ret;
}

int syscall_close_file(int fd) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_CLOSE_FILE), "b"(fd)
    );
    return 0;
}

int syscall_read_file(int fd, char* buffer, int num_bytes) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_READ_FILE), "b"(fd), "c"(buffer), "d"(num_bytes)
    );
    return ret;
}

int syscall_get_file_size(int fd) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_GET_FILE_SIZE), "b"(fd)
    );
    return ret;
}

int syscall_get_file_offset(int fd) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_GET_FILE_OFFSET), "b"(fd)
    );
    return ret;
}

int syscall_set_file_offset(int fd, uint32_t offset) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_SET_FILE_OFFSET), "b"(fd), "c"(offset)
    );
    return ret;
}

int syscall_open_dir(const char* path) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_OPEN_DIR), "b"(path)
    );
    return ret;
}

int syscall_close_dir() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_CLOSE_DIR)
    );
    return ret;
}

int syscall_next_file_in_dir(OSFileInfo* info) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_NEXT_FILE_IN_DIR), "b"(info)
    );
    return ret;
}

int syscall_create_shared_mem(int size) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_SHMEM_CREATE), "b"(size)
    );
    return ret;
}

int syscall_destroy_shared_mem(int id) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SHMEM_DESTROY), "b"(id)
    );
    return 0;
}

int syscall_shared_mem_exists(int id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_SHMEM_EXISTS), "b"(id)
    );
    return ret;
}

void syscall_unmap_shared_mem(int id) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SHMEM_UNMAP), "b"(id)
    );
    return;
}

void* syscall_map_shared_mem(int id) {
    void* addr;
    asm volatile(
        "int $0x80"
        : "=a"(addr)
        : "a"(SYSCALL_SHMEM_MAP), "b"(id)
    );
    return addr;
}

os_errorcode syscall_get_task_info(OSTaskInfo* list, uint32_t list_max_size, uint32_t* num_tasks) {
    os_errorcode error;
    asm volatile(
        "int $0x80"
        : "=a"(error)
        : "a"(SYSCALL_GET_TASK_INFO), "b"(list), "c"(list_max_size), "d"(num_tasks)
    );
    return error;
}

uint32_t syscall_get_system_time() {
    uint32_t ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_GET_SYSTEM_TIME)
    );
    return ret;
}
