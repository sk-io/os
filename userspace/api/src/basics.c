#include "os.h"

#include "syscalls.h"

int32_t os_get_task_id() {
    return syscall_get_task_id();
}

void os_exit() {
    syscall_exit();
}

void os_print(const char* str) {
    syscall_print(str);
}

void os_print_char(char c) {
    syscall_print_char(c);
}

void os_exec(const char* path, const char* argv[]) {
    syscall_exec(path, argv);
}

uint32_t os_get_system_time() {
    return syscall_get_system_time();
}
