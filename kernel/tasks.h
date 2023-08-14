#pragma once

#include "types.h"
#include "ramdisk.h"

#define MAX_TASKS 16
#define MAX_OPEN_FILES 16

// used for newly created tasks
typedef struct {
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 eip;
} TaskReturnContext;

enum {
    TASK_STATE_DEAD,
    TASK_STATE_READY,
    TASK_STATE_IDLE,
    TASK_STATE_WAIT_FOR_EVENT,
    TASK_STATE_WAIT_FOR_REDRAW,
};

typedef struct {
    u32 id;

    // kernel stack
    u32 kesp; // exchanged on task switch
    u32 kesp0; // top of kernel stack (highest address). copied to TSS, unused in kernel threads

    // page directory
    u32* pagedir;
    u32 is_kernel_task; // flags
    u32 state;
    FIL open_files[MAX_OPEN_FILES];
    void* event_buffer; // address in kernel vmem
    s32 event_shmem_id;
    u8 sharedmem_bitmap[8192]; // 1mb / 0x1000 bytes per page / 8 bits per byte

    u32 heap_start; // page aligned
    u32 heap_end;
} Task;

extern Task tasks[MAX_TASKS];
extern Task* current_task;
extern int num_tasks;

void setup_tasks();
s32 create_user_task(const char* path);
void create_kernel_task(void* func);
void kill_task(u32 id);
void task_schedule();
Task* get_task(int id);
