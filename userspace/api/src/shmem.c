#include "os.h"

#include "syscalls.h"

int32_t os_create_shared_mem(int32_t size) {
    syscall_create_shared_mem(size);
    return 0;
}

int32_t os_destroy_shared_mem(int32_t id) {
    syscall_destroy_shared_mem(id);
    return 0;
}

int32_t os_shared_mem_exists(int32_t id) {
    return syscall_shared_mem_exists(id);
}

void os_unmap_shared_mem(int32_t id) {
    syscall_unmap_shared_mem(id);
}

void* os_map_shared_mem(int32_t id) {
    return syscall_map_shared_mem(id);
}
