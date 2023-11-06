#include "os.h"

#include "syscalls.h"
#include "shmem.h"

int os_create_window(int width, int height, unsigned int flags) {
    return syscall_create_window(width, height, flags);
}

int os_destroy_window(int window_id) {
    return syscall_destroy_window(window_id);
}

void* os_map_window_framebuffer(int window_id) {
    int id = syscall_get_window_fb_shmem_id(window_id);
    return syscall_map_shared_mem(id);
}

// returns shown buffer index
// rename to os_redraw_window?
int os_swap_window_buffers(int window_id) {
    return syscall_swap_window_buffers(window_id);
}

int os_set_window_title(int window_id, const char* title) {
    return syscall_set_window_title(window_id, title);
}
