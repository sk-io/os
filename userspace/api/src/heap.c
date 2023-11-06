#include "os.h"

#include "syscalls.h"

uint32_t os_get_heap_start() {
    return syscall_get_heap_start();
}

uint32_t os_get_heap_end() {
    return syscall_get_heap_end();
}

void os_set_heap_end(uint32_t heap_end) {
    syscall_set_heap_end(heap_end);
}
