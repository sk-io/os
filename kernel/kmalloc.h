#pragma once

#include "types.h"

void kmalloc_init(u32 heap_size);
void* kmalloc(u32 bytes);
void kfree(void* addr);
u32 kmalloc_get_total_bytes();
