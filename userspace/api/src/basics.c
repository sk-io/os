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

