#include "os.h"

#include "../../../kernel/syscall_list.h"

int os_get_task_id() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_TASK_ID)
    );
    return ret;
}

void os_exit() {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_EXIT)
    );
}

void os_print(const char* str) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_PRINT), "b"(str)
    );
}

void os_exec(const char* path) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_EXEC), "b"(path)
    );
}

int os_open_file(const char* path) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_OPEN_FILE), "b"(path)
    );
    return ret;
}

int os_close_file(int fd) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_CLOSE_FILE), "b"(fd)
    );
    return 0;
}

int os_read_file(int fd, char* buffer, int num_bytes) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_READ_FILE), "b"(fd), "c"(buffer), "d"(num_bytes)
    );
    return ret;
}

int os_get_file_size(int fd) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_GET_FILE_SIZE)
    );
    return ret;
}
