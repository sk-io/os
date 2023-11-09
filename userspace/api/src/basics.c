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

void os_exec(const char* path) {
    syscall_exec(path);
}
