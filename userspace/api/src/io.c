#include "os.h"

#include "syscalls.h"

int32_t os_open_file(const char* path) {
    return syscall_open_file(path);
}

int32_t os_close_file(int32_t fd) {
    syscall_close_file(fd);
    return 0;
}

int32_t os_read_file(int32_t fd, char* buffer, int32_t num_bytes) {
    return syscall_read_file(fd, buffer, num_bytes);
}

int32_t os_get_file_size(int32_t fd) {
    return syscall_get_file_size(fd);
}

uint32_t os_get_file_offset(int32_t fd) {
    return syscall_get_file_offset(fd);
}

uint32_t os_set_file_offset(int32_t fd, uint32_t offset) {
    return syscall_set_file_offset(fd, offset);
}

int32_t os_open_dir(const char* path) {
    return syscall_open_dir(path);
}

int32_t os_close_dir() {
    return syscall_close_dir();
}

int32_t os_next_file_in_dir(OSFileInfo* info) {
    return syscall_next_file_in_dir(info);
}
