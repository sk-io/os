#include "os.h"

#include "../../../kernel/syscall_list.h"

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

int os_open_dir(const char* path) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_OPEN_DIR), "b"(path)
    );
    return ret;
}

int os_close_dir() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_CLOSE_DIR)
    );
    return ret;
}

int os_next_file_in_dir(OSFileInfo* info) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret) : "a"(SYSCALL_NEXT_FILE_IN_DIR), "b"(info)
    );
    return ret;
}
