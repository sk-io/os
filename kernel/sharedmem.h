#pragma once

#include "types.h"

typedef struct {
    u32 size_in_pages;

    u32* physical_pages;
} SharedMemory; // SharedMemoryObject

void sharedmem_init();
s32 sharedmem_create(u32 size);
void sharedmem_destroy(s32 id);
bool sharedmem_exists(s32 id);
void* sharedmem_map(s32 id, bool map_to_kernel);
void sharedmem_unmap(s32 id, void* vaddr);
