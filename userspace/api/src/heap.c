#include "os.h"

#include "../../../kernel/syscall_list.h"

uint32_t os_get_heap_start() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_HEAP_START)
    );
    return ret;
}

uint32_t os_get_heap_end() {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret) : "a"(SYSCALL_GET_HEAP_END)
    );
    return ret;
}

void os_set_heap_end(uint32_t heap_end) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SET_HEAP_END), "b"(heap_end)
    );
}
