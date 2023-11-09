#include "os.h"

#include "syscalls.h"

int32_t os_create_window(int32_t width, int32_t height, uint32_t flags) {
    return syscall_create_window(width, height, flags);
}

int32_t os_destroy_window(int32_t window_id) {
    return syscall_destroy_window(window_id);
}

void* os_map_window_framebuffer(int32_t window_id) {
    int32_t id = syscall_get_window_fb_shmem_id(window_id);
    return syscall_map_shared_mem(id);
}

// returns shown buffer index
// rename to os_redraw_window?
int32_t os_swap_window_buffers(int32_t window_id) {
    return syscall_swap_window_buffers(window_id);
}

int32_t os_set_window_title(int32_t window_id, const char* title) {
    return syscall_set_window_title(window_id, title);
}
