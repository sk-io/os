#pragma once

// physical memory manager

#include "types.h"

void pmm_init(u32 mem_low, u32 mem_high);
u32 pmm_alloc_pageframe();
void pmm_free_pageframe(u32 addr);
u32 pmm_get_total_allocated_pages();
