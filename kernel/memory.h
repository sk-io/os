#pragma once

#include "types.h"

/*
    Virtual Address Space:
    0 - user start


    0xB000 0000 - user stack top
    0xB000 0000 - shared memory
    0xC000 0000 - kernel start
*/
#define USER_STACK_PAGES (0x10)

#define USER_STACK_BOTTOM    0xB0000000
#define USER_SHARED_MEMORY   0xB0000000
#define KERNEL_START         0xC0000000
#define KERNEL_GFX           0xC8000000
#define KERNEL_MALLOC        0xD0000000
#define KERNEL_SHARED_MEMORY 0xF0000000

#define PAGE_FLAG_PRESENT (1 << 0)
#define PAGE_FLAG_WRITE   (1 << 1)
#define PAGE_FLAG_USER    (1 << 2)
#define PAGE_FLAG_4MB     (1 << 7)
#define PAGE_FLAG_OWNER   (1 << 9) // means we are in charge of the physical page

#define P_PHYS_ADDR(x) ((x) & ~0xFFF)

// note that these are virtual addresses!
#define REC_PAGEDIR ((u32*) 0xFFFFF000)
#define REC_PAGETABLE(i) ((u32*) (0xFFC00000 + ((i) << 12)))

extern u32 initial_page_dir[1024];
extern int mem_num_vpages;

void init_memory(u32 mem_high);
void mem_map_page(u32 virt_addr, u32 phys_addr, u32 flags);
u32 mem_unmap_page(u32 virt_addr);
u32* mem_alloc_page_dir();
void mem_free_page_dir(u32* page_dir);
void mem_change_page_directory(u32* pd);
u32* mem_get_current_page_directory();
