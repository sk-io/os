#pragma once

#include <stdint.h>
#include <stdarg.h>

// a single, unified API for:
// thread/process control
// file IO
// window management
// audio

typedef uint32_t os_errorcode;

#define OS_EXPORT __attribute__((visibility("default")))

OS_EXPORT void os_temp_init(); // todo: remove this?

// basics
OS_EXPORT int32_t os_get_task_id();
OS_EXPORT void os_print(const char* msg);
OS_EXPORT void os_print_char(char c);
OS_EXPORT void os_exec(const char* path, const char* argv[]);
OS_EXPORT void os_exit();
OS_EXPORT uint32_t os_get_system_time();

// file i/o
typedef struct {
    uint32_t attributes;
    char name[256];
} OSFileInfo;
#define OS_FILE_INFO_IS_DIR 16

OS_EXPORT int32_t os_open_file(const char* path);
OS_EXPORT int32_t os_close_file(int32_t fd);
OS_EXPORT int32_t os_read_file(int32_t fd, char* buffer, int32_t num_bytes);
OS_EXPORT int32_t os_write_file(int32_t fd, const char* buffer, int32_t num_bytes);
OS_EXPORT int32_t os_get_file_size(int32_t fd);
OS_EXPORT uint32_t os_get_file_offset(int32_t fd);
OS_EXPORT uint32_t os_set_file_offset(int32_t fd, uint32_t offset);
OS_EXPORT int32_t os_open_dir(const char* path);
OS_EXPORT int32_t os_close_dir();
OS_EXPORT int32_t os_next_file_in_dir(OSFileInfo* info);

// heap
OS_EXPORT uint32_t os_get_heap_start();
OS_EXPORT uint32_t os_get_heap_end();
OS_EXPORT void os_set_heap_end(uint32_t heap_end);
OS_EXPORT void* os_malloc(uint32_t size);
OS_EXPORT void os_free(void* addr);

// window
#define OS_FULLSCREEN (1 << 0)
#define OS_DOUBLE_BUFFERED (1 << 1)
#define OS_FRAMEBUFFER_TRANSPARENCY (1 << 2)

OS_EXPORT int32_t os_create_window(int32_t width, int32_t height, uint32_t flags);
OS_EXPORT int32_t os_destroy_window(int32_t window_id);
OS_EXPORT void* os_map_window_framebuffer(int32_t window_id);
OS_EXPORT int32_t os_swap_window_buffers(int32_t window_id);
OS_EXPORT int32_t os_set_window_title(int32_t window_id, const char* title);

// shared memory
OS_EXPORT int32_t os_create_shared_mem(int32_t size);
OS_EXPORT int32_t os_destroy_shared_mem(int32_t id);
OS_EXPORT int32_t os_shared_mem_exists(int32_t id);
OS_EXPORT void* os_map_shared_mem(int32_t id);
OS_EXPORT void os_unmap_shared_mem(int32_t id);

// events
enum {
    OS_EVENT_NONE,
    OS_EVENT_KEYBOARD,
    OS_EVENT_MOUSE_CLICK,
    OS_EVENT_MOUSE_MOVE,
    OS_EVENT_TIMER,
};

typedef struct {
    uint32_t type;
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
} __attribute__((__packed__)) OSEvent;

#define OS_SHIFT_HELD (1 << 0)
#define OS_CTRL_HELD  (1 << 1)
#define OS_ALT_HELD   (1 << 2)

typedef struct {
    uint32_t type;
    uint32_t scancode;
    uint16_t flags;
    uint8_t ascii;
    uint8_t state;
} __attribute__((__packed__)) OSKeyboardEvent;

typedef struct {
    uint32_t type;
    uint32_t x;
    uint32_t y;
    uint32_t buttons;
} __attribute__((__packed__)) OSMouseEvent;

typedef struct {
    uint32_t type;
    uint32_t timer_id;
    uint32_t time_of_fire;
    uint32_t data2;
} __attribute__((__packed__)) OSTimerEvent;

OS_EXPORT int32_t os_poll_event(OSEvent* event);
OS_EXPORT int32_t os_wait_for_events();
OS_EXPORT void os_set_timer_interval(int32_t timer_id, int32_t interval_ms);

// utils
OS_EXPORT void os_printf(const char* msg, ...);
OS_EXPORT void os_vprintf(const char* format, va_list args);

// system info
typedef struct {
    uint32_t id;
    uint32_t state;
} OSTaskInfo;

OS_EXPORT os_errorcode os_get_task_info(OSTaskInfo* list, uint32_t list_max_size, uint32_t* num_tasks);
