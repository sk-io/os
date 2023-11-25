#include "memory.h"

#include "util.h"
#include "interrupts.h"
#include "log.h"
#include "physalloc.h"
#include "kmalloc.h"

// rename to paging.c ?

#define NUM_PAGE_DIRS 256
static u32 page_dirs[NUM_PAGE_DIRS][1024] __attribute__((aligned(4096)));
static u8 page_dir_used[NUM_PAGE_DIRS]; // todo: use bitfield

int mem_num_vpages;

static void enable_paging();
static void invalidate(int vaddr);
static void sync_page_dirs();

void init_memory(u32 mem_high, u32 phys_alloc_start) {
    mem_num_vpages = 0;

    // unmap the first 4 mb
    initial_page_dir[0] = 0;
    invalidate(0);

    // recursive table mapping
    initial_page_dir[1023] = ((u32) initial_page_dir - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
    invalidate(0xFFFFF000);

    pmm_init(phys_alloc_start, mem_high);

    memset(page_dirs, 0, 0x1000 * NUM_PAGE_DIRS);
    memset(page_dir_used, 0, NUM_PAGE_DIRS);
}

void mem_change_page_directory(u32* pd) {
    assert_msg((u32) pd > KERNEL_START, "whats up");
    pd = (u32*) (((u32) pd) - KERNEL_START); // calc the physical address
    asm volatile(
        "mov %0, %%eax \n"
        "mov %%eax, %%cr3 \n"
        :: "m"(pd)
    );
}

u32* mem_get_current_page_directory() {
    u32 pd;
    asm volatile(
        "mov %%cr3, %0"
        : "=r"(pd)
    );
    pd += KERNEL_START; // calc the virtual address
    return (u32*) pd;
}

void enable_paging() {
    asm volatile(
        "mov %cr4, %ebx \n"
        "or $0x10, %ebx \n"
        "mov %ebx, %cr4 \n"

        "mov %cr0, %ebx \n"
        "or $0x80000000, %ebx \n"
        "mov %ebx, %cr0 \n"
    );
}

void invalidate(int vaddr) {
    asm volatile("invlpg %0" :: "m"(vaddr));
}

// addresses need to be 4096 aligned
void mem_map_page(u32 virt_addr, u32 phys_addr, u32 flags) {
    VERIFY_INTERRUPTS_DISABLED;
    // kernel_log("pdir=%x mem_map_page: %x mapped to %x", mem_get_current_page_directory(), virt_addr, phys_addr);

    u32* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) {
        // optimization: just copy from current pagedir to all others, including init_page_dir
        //              we might just wanna have init_page_dir be page_dirs[0] instead
        //              then we would have to build it in boot.asm in assembly
        
        // write to initial_page_dir, so that we can sync that across all others

        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(initial_page_dir);
    }

    // extract indices from the vaddr
    u32 pd_index = virt_addr >> 22;
    u32 pt_index = virt_addr >> 12 & 0x3FF;

    u32* page_dir = REC_PAGEDIR;

    // page tables can only be directly accessed/modified using the recursive strat?
    // > yes since their physical page is not mapped into memory
    u32* pt = REC_PAGETABLE(pd_index);

    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        // allocate a page table
        u32 pt_paddr = pmm_alloc_pageframe();
        // kernel_log("creating table %x", pt_paddr);

        page_dir[pd_index] = pt_paddr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_OWNER | flags;
        invalidate(virt_addr);

        // we can now access it directly using the recursive strategy
        for (u32 i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
    }

    pt[pt_index] = phys_addr | PAGE_FLAG_PRESENT | flags;
    mem_num_vpages++;
    invalidate(virt_addr);

    if (prev_page_dir != 0) {
        // ... then sync that across all others
        sync_page_dirs();
        // we changed to init page dir, now we need to change back
        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(prev_page_dir);
    }
}

// returns page table entry (physical address and flags)
u32 mem_unmap_page(u32 virt_addr) {
    VERIFY_INTERRUPTS_DISABLED;

    u32* prev_page_dir = 0;
    if (virt_addr >= KERNEL_START) {
        // optimization: just copy from current pagedir to all others, including init_page_dir
        //              we might just wanna have init_page_dir be page_dirs[0] instead
        //              then we would have to build it in boot.asm in assembly
        
        // write to initial_page_dir, so that we can sync that across all others

        // kernel_log("KERNEL SPACE !!!!!!!!!!");

        prev_page_dir = mem_get_current_page_directory();

        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(initial_page_dir);
    }

    u32 pd_index = virt_addr >> 22;
    u32 pt_index = virt_addr >> 12 & 0x3FF;

    u32* page_dir = REC_PAGEDIR;

    u32 pd_entry = page_dir[pd_index];
    assert_msg(pd_entry & PAGE_FLAG_PRESENT, "tried to free page from a non present page table?");

    u32* pt = REC_PAGETABLE(pd_index);

    u32 pte = pt[pt_index];
    assert_msg(pte & PAGE_FLAG_PRESENT, "tried to free non present page");

    pt[pt_index] = 0;

    mem_num_vpages--;

    bool remove = true;
    // optimization: keep track of the number of pages present in each page table
    for (u32 i = 0; i < 1024; i++) {
        if (pt[i] & PAGE_FLAG_PRESENT) {
            remove = false;
            break;
        }
    }

    if (remove) {
        // table is empty, destroy its physical frame if we own it.
        u32 pde = page_dir[pd_index];
        if (pde & PAGE_FLAG_OWNER) {
            // kernel_log("REMOVING PAGETABLE");
            u32 pt_paddr = P_PHYS_ADDR(pde);
            // kernel_log("removing page table %x", pt_paddr);
            pmm_free_pageframe(pt_paddr);
            page_dir[pd_index] = 0;
        }
    }

    invalidate(virt_addr);

    // free it here for now
    if (pte & PAGE_FLAG_OWNER) {
        pmm_free_pageframe(P_PHYS_ADDR(pte));
    }

    if (prev_page_dir != 0) {
        // ... then sync that across all others
        sync_page_dirs();
        // we changed to init page dir, now we need to change back
        if (prev_page_dir != initial_page_dir)
            mem_change_page_directory(prev_page_dir);
    }

    return pte;
}

u32* mem_alloc_page_dir() {
    VERIFY_INTERRUPTS_DISABLED;

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (!page_dir_used[i]) {
            page_dir_used[i] = true;

            u32* page_dir = page_dirs[i];
            memset(page_dir, 0, 0x1000);

            // first 768 entries are user page tables
            for (int i = 0; i < 768; i++) {
                page_dir[i] = 0;
            }

            // next 256 are kernel (except last)
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER; // we don't own these though
            }

            // recursive mapping
            page_dir[1023] = (((u32) page_dir) - KERNEL_START) | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE;
            return page_dir;
        }
    }

    assert_msg(false, "no page dirs left!");
    return 0;
}

void mem_free_page_dir(u32* page_dir) {
    VERIFY_INTERRUPTS_DISABLED;

    u32* prev_pagedir = mem_get_current_page_directory();
    mem_change_page_directory(page_dir);

    u32 pagedir_index = ((u32)page_dir) - ((u32) page_dirs);
    pagedir_index /= 4096;

    // kernel_log("pagedir_index = %u", pagedir_index);

    u32* pd = REC_PAGEDIR;
    for (int i = 0; i < 768; i++) {
        int pde = pd[i];
        if (pde == 0)
            continue;
        
        u32* ptable = REC_PAGETABLE(i);
        for (int j = 0; j < 1024; j++) {
            u32 pte = ptable[j];

            if (pte & PAGE_FLAG_OWNER) {
                pmm_free_pageframe(P_PHYS_ADDR(pte));
            }
        }
        memset(ptable, 0, 4096); // is this necessary?

        if (pde & PAGE_FLAG_OWNER) {
            // we created this pagetable, lets free it now
            pmm_free_pageframe(P_PHYS_ADDR(pde));
        }
        pd[i] = 0;
    }

    // do we need to clean the kernel portion?

    page_dir_used[pagedir_index] = 0;
    mem_change_page_directory(prev_pagedir);
}

// sync the kernel portions of all in-use pagedirs
void sync_page_dirs() {
    VERIFY_INTERRUPTS_DISABLED;

    for (int i = 0; i < NUM_PAGE_DIRS; i++) {
        if (page_dir_used[i]) {
            u32* page_dir = page_dirs[i];
            
            for (int i = 768; i < 1023; i++) {
                page_dir[i] = initial_page_dir[i] & ~PAGE_FLAG_OWNER; // we don't own these though
            }
        }
    }
}

u32 mem_get_phys_from_virt(u32 virt_addr) {
    VERIFY_INTERRUPTS_DISABLED;

    u32 pd_index = virt_addr >> 22;
    u32* page_dir = REC_PAGEDIR;
    if (!(page_dir[pd_index] & PAGE_FLAG_PRESENT)) {
        return -1;
    }

    u32 pt_index = virt_addr >> 12 & 0x3FF;
    u32* pt = REC_PAGETABLE(pd_index);

    return P_PHYS_ADDR(pt[pt_index]);
}

bool mem_is_valid_vaddr(u32 vaddr) {
    VERIFY_INTERRUPTS_DISABLED;
    
    u32 pd_index = vaddr >> 22;
    u32 pt_index = vaddr >> 12 & 0x3FF;

    u32* page_dir = REC_PAGEDIR;

    u32 pd_entry = page_dir[pd_index];
    if (!(pd_entry & PAGE_FLAG_PRESENT))
        return false;
    
    u32* pt = REC_PAGETABLE(pd_index);

    u32 pt_entry = pt[pt_index];
    return pt_entry & PAGE_FLAG_PRESENT;
}
