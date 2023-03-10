#include "os.h"

#include "../../../kernel/syscall_list.h"

int os_create_shared_mem(int id, int size) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SHMEM_CREATE), "b"(id), "c"(size)
    );
    return 0;
}

int os_destroy_shared_mem(int id) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SHMEM_DESTROY), "b"(id)
    );
    return 0;
}

int os_shared_mem_exists(int id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_SHMEM_EXISTS), "b"(id)
    );
    return ret;
}

void* os_map_shared_mem(int id) {
    void* addr;
    asm volatile(
        "int $0x80"
        : "=a"(addr)
        : "a"(SYSCALL_SHMEM_MAP), "b"(id), "c"(0)
    );
    return addr;
}

void os_unmap_shared_mem(int id) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SHMEM_UNMAP), "b"(id)
    );
    return 0;
}
