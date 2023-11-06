#pragma once

#include "os.h"

int syscall_get_task_id();
void syscall_exit();
void syscall_print(const char* str);
void syscall_exec(const char* path);
int syscall_get_event_buffer_shmem_id();
void syscall_wait_for_events();
void syscall_set_timer_interval(int timer_id, int interval_ms);
int syscall_create_window(int width, int height, unsigned int flags);
int syscall_destroy_window(int window_id);
int syscall_get_window_fb_shmem_id(int window_id);
int syscall_swap_window_buffers(int window_id);
int syscall_set_window_title(int window_id, const char* title);
uint32_t syscall_get_heap_start();
uint32_t syscall_get_heap_end();
void syscall_set_heap_end(uint32_t heap_end);
int syscall_open_file(const char* path);
int syscall_close_file(int fd);
int syscall_read_file(int fd, char* buffer, int num_bytes);
int syscall_get_file_size(int fd);
int syscall_open_dir(const char* path);
int syscall_close_dir();
int syscall_next_file_in_dir(OSFileInfo* info);
int syscall_create_shared_mem(int id, int size);
int syscall_destroy_shared_mem(int id);
int syscall_shared_mem_exists(int id);
void syscall_unmap_shared_mem(int id);
void* syscall_map_shared_mem(int id);
