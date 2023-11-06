#pragma once

#include <stdint.h>

// a single, unified API for:
// thread/process control
// file IO
// window management
// audio

#define OS_EXPORT __attribute__((visibility("default")))

OS_EXPORT void os_temp_init(); // todo: remove this

// basics
OS_EXPORT int os_get_task_id();
OS_EXPORT void os_print(const char* msg);
OS_EXPORT void os_exec(const char* path);
OS_EXPORT void os_exit();

// file i/o
typedef struct {
    uint32_t attributes;
    char name[256];
} OSFileInfo;
#define OS_FILE_INFO_IS_DIR 16

OS_EXPORT int os_open_file(const char* path);
OS_EXPORT int os_close_file(int fd);
OS_EXPORT int os_read_file(int fd, char* buffer, int num_bytes);
OS_EXPORT int os_get_file_size(int fd);
OS_EXPORT int os_open_dir(const char* path);
OS_EXPORT int os_close_dir();
OS_EXPORT int os_next_file_in_dir(OSFileInfo* info);

// heap
OS_EXPORT uint32_t os_get_heap_start();
OS_EXPORT uint32_t os_get_heap_end();
OS_EXPORT void os_set_heap_end(uint32_t heap_end);
OS_EXPORT void* os_malloc(uint32_t size);
OS_EXPORT void os_free(void* addr);

// window
#define OS_FULLSCREEN (1 << 0)
#define OS_DOUBLE_BUFFERED (1 << 1)

OS_EXPORT int os_create_window(int width, int height, unsigned int flags);
OS_EXPORT int os_destroy_window(int window_id);
OS_EXPORT void* os_map_window_framebuffer(int window_id);
OS_EXPORT int os_swap_window_buffers(int window_id);
OS_EXPORT int os_set_window_title(int window_id, const char* title);

// shared memory
OS_EXPORT int os_create_shared_mem(int id, int size);
OS_EXPORT int os_destroy_shared_mem(int id);
OS_EXPORT int os_shared_mem_exists(int id);
OS_EXPORT void* os_map_shared_mem(int id);
OS_EXPORT void os_unmap_shared_mem(int id);

// events
enum {
    OS_EVENT_NONE,
    OS_EVENT_KEYBOARD,
    OS_EVENT_MOUSE_CLICK,
    OS_EVENT_MOUSE_MOVE,
    OS_EVENT_TIMER,
};

typedef struct {
    unsigned int type;
    unsigned int data0;
    unsigned int data1;
    unsigned int data2;
} OSEvent;

typedef struct {
    unsigned int type;
    unsigned int scancode;
    unsigned int ascii;
    unsigned int state;
} OSKeyboardEvent;

typedef struct {
    unsigned int type;
    unsigned int x;
    unsigned int y;
    unsigned int buttons;
} OSMouseEvent;

typedef struct {
    unsigned int type;
    unsigned int timer_id;
    unsigned int time_of_fire;
    unsigned int data2;
} OSTimerEvent;

OS_EXPORT int os_poll_event(OSEvent* event);
OS_EXPORT int os_wait_for_events();
OS_EXPORT void os_set_timer_interval(int timer_id, int interval_ms);

// utils
OS_EXPORT void os_printf(const char* msg, ...);
