#pragma once

#include "types.h"
#include "slib.h"
#include "config.h"
#include "sharedmem.h"
#include "fat32.h"

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
    u32 interval;
    u64 next_fire;
    bool active;
} Timer;

typedef struct {
    u32 id;

    // kernel stack
    u32 kesp; // exchanged on task switch
    u32 kesp0; // top of kernel stack (highest address) ...
    // ... stack ptr is set to this (via TSS) when transitioning from user to kernel mode (on interrupt/syscall)

    u32* pagedir; // this task's page directory, kernel tasks use the initial pagedir
    u32 is_kernel_task;
    u32 state; // task state enum
    FAT32_File open_files[MAX_OPEN_FILES]; // hack
    void* event_buffer; // address to shared memory in kernel vmem (kernel-side)
    s32 event_shmem_id; // used to share the buffer with userspace
    SharedMemoryMappingPool shmem;

    u32 heap_start; // page aligned
    u32 heap_end;

    // DIR open_dir;

    Timer timers[MAX_TIMERS];
    OpenSharedLibrary slibs[MAX_SHARED_LIBS_PER_TASK];
} Task;

extern Task tasks[MAX_TASKS];
extern Task* current_task;
extern int num_tasks;

void setup_tasks();
s32 create_user_task(const char* path, const char* argv[]);
s32 create_kernel_task(void* func);
void kill_task(u32 id);
void task_schedule();
Task* get_task(int id);
