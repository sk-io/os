#include "os.h"

#include "syscalls.h"
#include "shmem.h"

int os_create_shared_mem(int id, int size) {
    syscall_create_shared_mem(id, size);
    return 0;
}

int os_destroy_shared_mem(int id) {
    syscall_destroy_shared_mem(id);
    return 0;
}

int os_shared_mem_exists(int id) {
    return syscall_shared_mem_exists(id);
}

void os_unmap_shared_mem(int id) {
    syscall_unmap_shared_mem(id);
}

void* os_map_shared_mem(int id) {
    return syscall_map_shared_mem(id);
}
