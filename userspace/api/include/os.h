#pragma once

#include <stdint.h>

// a single, unified API for:
// thread/process control
// file IO
// window management
// audio

// basics
int os_get_task_id();
void os_print(const char* msg);
void os_exec(const char* path);
void os_exit();

// file i/o
typedef struct {
    uint32_t attributes;
    char name[256];
} OSFileInfo;
#define OS_FILE_INFO_IS_DIR 16

int os_open_file(const char* path);
int os_close_file(int fd);
int os_read_file(int fd, char* buffer, int num_bytes);
int os_get_file_size(int fd);
int os_open_dir(const char* path);
int os_close_dir();
int os_next_file_in_dir(OSFileInfo* info);

// heap
uint32_t os_get_heap_start();
uint32_t os_get_heap_end();
void os_set_heap_end(uint32_t heap_end);
void* os_malloc(uint32_t size);
void os_free(void* addr);

// window
#define OS_FULLSCREEN (1 << 0)
#define OS_DOUBLE_BUFFERED (1 << 1)

int os_create_window(int width, int height, unsigned int flags);
int os_destroy_window(int window_id);
void* os_map_window_framebuffer(int window_id);
int os_swap_window_buffers(int window_id);
int os_set_window_title(int window_id, const char* title);

// shared memory
int os_create_shared_mem(int id, int size);
int os_destroy_shared_mem(int id);
int os_shared_mem_exists(int id);
void* os_map_shared_mem(int id);
void os_unmap_shared_mem(int id);

// events
enum {
    EVENT_NONE,
    EVENT_KEYBOARD,
    EVENT_MOUSE,
    EVENT_MOUSE_MOVE,
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

int os_poll_event(OSEvent* event);
int os_wait_for_events();

// utils
void os_printf(const char* msg, ...);
