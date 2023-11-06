#include "os.h"

#include "syscalls.h"

int os_open_file(const char* path) {
    return syscall_open_file(path);
}

int os_close_file(int fd) {
    syscall_close_file(fd);
    return 0;
}

int os_read_file(int fd, char* buffer, int num_bytes) {
    return syscall_read_file(fd, buffer, num_bytes);
}

int os_get_file_size(int fd) {
    return syscall_get_file_size(fd);
}

int os_open_dir(const char* path) {
    return syscall_open_dir(path);
}

int os_close_dir() {
    return syscall_close_dir();
}

int os_next_file_in_dir(OSFileInfo* info) {
    return syscall_next_file_in_dir(info);
}
