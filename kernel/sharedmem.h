#pragma once

#include "types.h"
#include "config.h"

typedef struct {
    u32 size_in_pages;
    u32* physical_pages;
    u32 owner_task_id; // 0 means kernel owned
} SharedMemory; // SharedMemoryObject?

typedef struct {
    u32 vaddr;
    u32 shmem_obj;
} SharedMemoryMapping; // MOVES AROUND, DONT STORE REFERENCES TO THIS DIRECTLY!

// TODO: dynamic array
typedef struct {
    SharedMemoryMapping mappings[MAX_SHARED_MEMORY_MAPPINGS];
    int num;
    u32 vaddr_start;
} SharedMemoryMappingPool;

extern SharedMemory shmem[MAX_SHARED_MEMORY_OBJS];

void sharedmem_init();
s32 sharedmem_create(u32 size, u32 owner_task_id);
void sharedmem_destroy(s32 id);
bool sharedmem_exists(s32 id);
void* sharedmem_map(s32 id, u32 task_id);
void sharedmem_unmap(s32 id, u32 task_id);
void shmem_print_mappings(SharedMemoryMappingPool* pool);
