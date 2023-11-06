#include "os.h"

#include "../../../kernel/syscall_list.h"
#include "shmem.h"

int os_create_window(int width, int height, unsigned int flags) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a"(SYSCALL_CREATE_WINDOW), "b"(width), "c"(height), "d"(flags)
    );
    return ret;
}

int os_destroy_window(int window_id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a" (ret)
        : "a"(SYSCALL_CREATE_WINDOW), "b"(window_id)
    );
    return ret;
}

void* os_map_window_framebuffer(int window_id) {
    int id;
    asm volatile(
        "int $0x80"
        : "=a" (id)
        : "a"(SYSCALL_GET_WINDOW_FB_SHMEM_ID), "b"(window_id)
    );

    return map_shared_mem(id);
}

// returns shown buffer index
// rename to os_redraw_window?
int os_swap_window_buffers(int window_id) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_WINDOW_SWAP_BUFFERS), "b"(window_id)
    );
    return ret;
}

int os_set_window_title(int window_id, const char* title) {
    int ret;
    asm volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"(SYSCALL_SET_WINDOW_TITLE), "b"(window_id), "c"(title)
    );
    return ret;
}
