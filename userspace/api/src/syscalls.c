#include "os.h"

#include "../../../kernel/syscall_list.h"

static int32_t _syscall_0(uint32_t syscall) {
    int32_t ret;
    asm volatile("int $0x80" : "=a" (ret) : "a"(syscall));
    return ret;
}

static int32_t _syscall_1(uint32_t syscall, int32_t arg0) {
    int32_t ret;
    asm volatile("int $0x80" : "=a" (ret) : "a"(syscall), "b"(arg0));
    return ret;
}

static int32_t _syscall_2(uint32_t syscall, int32_t arg0, int32_t arg1) {
    int32_t ret;
    asm volatile("int $0x80" : "=a" (ret) : "a"(syscall), "b"(arg0), "c"(arg1));
    return ret;
}

static int32_t _syscall_3(uint32_t syscall, int32_t arg0, int32_t arg1, int32_t arg2) {
    int32_t ret;
    asm volatile("int $0x80" : "=a" (ret) : "a"(syscall), "b"(arg0), "c"(arg1), "d"(arg2));
    return ret;
}

int syscall_get_task_id() {
    return _syscall_0(SYSCALL_GET_TASK_ID);
}

void syscall_exit() {
    _syscall_0(SYSCALL_EXIT);
}

void syscall_print(const char* str) {
    _syscall_1(SYSCALL_PRINT, str);
}

void syscall_print_char(char c) {
    _syscall_1(SYSCALL_PRINT_CHAR, c);
}

void syscall_exec(const char* path, const char* argv[]) {
    _syscall_2(SYSCALL_EXEC, path, argv);
}

int syscall_get_event_buffer_shmem_id() {
    return _syscall_0(SYSCALL_GET_EVENT_BUFFER_SHMEM_ID);
}

void syscall_wait_for_events() {
    _syscall_0(SYSCALL_WAIT_FOR_EVENT);
}

void syscall_set_timer_interval(int timer_id, int interval_ms) {
    _syscall_2(SYSCALL_SET_TIMER_INTERVAL, timer_id, interval_ms);
}

int syscall_create_window(int width, int height, unsigned int flags) {
    return _syscall_3(SYSCALL_CREATE_WINDOW, width, height, flags);
}

int syscall_destroy_window(int window_id) {
    return _syscall_1(SYSCALL_CREATE_WINDOW, window_id);
}

int syscall_get_window_fb_shmem_id(int window_id) {
    return _syscall_1(SYSCALL_GET_WINDOW_FB_SHMEM_ID, window_id);
}

int syscall_swap_window_buffers(int window_id) {
    return _syscall_1(SYSCALL_WINDOW_SWAP_BUFFERS, window_id);
}

int syscall_set_window_title(int window_id, const char* title) {
    return _syscall_2(SYSCALL_SET_WINDOW_TITLE, window_id, title);
}

uint32_t syscall_get_heap_start() {
    return _syscall_0(SYSCALL_GET_HEAP_START);
}

uint32_t syscall_get_heap_end() {
    return _syscall_0(SYSCALL_GET_HEAP_END);
}

void syscall_set_heap_end(uint32_t heap_end) {
    _syscall_1(SYSCALL_SET_HEAP_END, heap_end);
}

int syscall_open_file(const char* path) {
    return _syscall_1(SYSCALL_OPEN_FILE, path);
}

int syscall_close_file(int fd) {
    _syscall_1(SYSCALL_CLOSE_FILE, fd);
    return 0; // FIXME: ?
}

int syscall_read_file(int fd, char* buffer, int num_bytes) {
    return _syscall_3(SYSCALL_READ_FILE, fd, buffer, num_bytes);
}

int syscall_write_file(int fd, char* buffer, int num_bytes) {
    return _syscall_3(SYSCALL_WRITE_FILE, fd, buffer, num_bytes);
}

int syscall_get_file_size(int fd) {
    return _syscall_1(SYSCALL_GET_FILE_SIZE, fd);
}

int syscall_get_file_offset(int fd) {
    return _syscall_1(SYSCALL_GET_FILE_SIZE, fd);
}

int syscall_set_file_offset(int fd, uint32_t offset) {
    return _syscall_2(SYSCALL_SET_FILE_OFFSET, fd, offset);
}

int syscall_open_dir(const char* path) {
    return _syscall_1(SYSCALL_OPEN_DIR, path);
}

int syscall_close_dir() {
    return _syscall_0(SYSCALL_CLOSE_DIR);
}

int syscall_next_file_in_dir(OSFileInfo* info) {
    return _syscall_1(SYSCALL_NEXT_FILE_IN_DIR, info);
}

int syscall_create_shared_mem(int size) {
    return _syscall_1(SYSCALL_SHMEM_CREATE, size);
}

int syscall_destroy_shared_mem(int id) {
    return _syscall_1(SYSCALL_SHMEM_DESTROY, id);
}

int syscall_shared_mem_exists(int id) {
    return _syscall_1(SYSCALL_SHMEM_EXISTS, id);
}

void syscall_unmap_shared_mem(int id) {
    _syscall_1(SYSCALL_SHMEM_UNMAP, id);
}

void* syscall_map_shared_mem(int id) {
    return _syscall_1(SYSCALL_SHMEM_MAP, id);
}

os_errorcode syscall_get_task_info(OSTaskInfo* list, uint32_t list_max_size, uint32_t* num_tasks) {
    return _syscall_3(SYSCALL_GET_TASK_INFO, list, list_max_size, num_tasks);
}

uint32_t syscall_get_system_time() {
    return _syscall_0(SYSCALL_GET_SYSTEM_TIME);
}
