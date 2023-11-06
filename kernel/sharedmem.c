#include "sharedmem.h"

#include "util.h"
#include "kmalloc.h"
#include "memory.h"
#include "physalloc.h"
#include "syscall.h"
#include "log.h"
#include "tasks.h"

#define MAX_SHAREDMEM_OBJS 32

static SharedMemory shmem[MAX_SHAREDMEM_OBJS];
u8 kernel_sharedmem_bitmap[8192];

static s32 find_available_shmem_slot();
static u32 find_available_virtual_region(u32 num_pages, bool map_to_kernel);
static void mark_pages_used_in_bitmap(u8* bitmap, u32 start, u32 num);
static void mark_pages_unused_in_bitmap(u8* bitmap, u32 start, u32 num);
static void* sharedmem_map_syscall(s32 id);

void sharedmem_init() {
    memset(kernel_sharedmem_bitmap, 0, sizeof(kernel_sharedmem_bitmap));
    memset(shmem, 0, sizeof(shmem));
    register_syscall(SYSCALL_SHMEM_CREATE, sharedmem_create);
    register_syscall(SYSCALL_SHMEM_DESTROY, sharedmem_destroy);
    register_syscall(SYSCALL_SHMEM_EXISTS, sharedmem_exists);
    register_syscall(SYSCALL_SHMEM_MAP, sharedmem_map_syscall);
    register_syscall(SYSCALL_SHMEM_UNMAP, sharedmem_unmap);
}

// problem: how do two processes agree on the same shmem obj id?
// should we specify here if we are the owner?
s32 sharedmem_create(u32 size) {
    // assert(!sharedmem_exists(id));
    assert(size);

    s32 id = find_available_shmem_slot();

    // kernel_log("sharedmem_create %u %u", id, size);

    u32 num_pages = CEIL_DIV(size, 0x1000);
    shmem[id].size_in_pages = num_pages;
    // kernel_log("num_pages %u", shmem[id].size_in_pages);
    shmem[id].physical_pages = kmalloc(num_pages * sizeof(u32));

    for (int i = 0; i < num_pages; i++) {
        shmem[id].physical_pages[i] = pmm_alloc_pageframe();
    }

    return id;
}

void sharedmem_destroy(s32 id) {
    assert(sharedmem_exists(id));

    for (int i = 0; i < shmem[id].size_in_pages; i++) {
        pmm_free_pageframe(shmem[id].physical_pages[i]);
    }

    memset(&shmem[id], 0, sizeof(SharedMemory));
}

bool sharedmem_exists(s32 id) {
    if (id >= MAX_SHAREDMEM_OBJS)
        return false;
    return shmem[id].size_in_pages != 0;
}

// should we specify here if we are the owner?
void* sharedmem_map(s32 id, bool map_to_kernel) {
    assert(sharedmem_exists(id));

    SharedMemory* obj = &shmem[id];

    u32 required = obj->size_in_pages;
    u32 vaddr = find_available_virtual_region(required, map_to_kernel);
    // kernel_log("id=%u vaddr=%x map_to_kernel=%u", id, vaddr, map_to_kernel);

    for (int i = 0; i < required; i++) {
        // kernel_log("sharedmem: mapping %x to %x", vaddr + i * 0x1000, obj->physical_pages[i]);
        u32 flags = PAGE_FLAG_WRITE;
        if (!map_to_kernel)
            flags |= PAGE_FLAG_USER;
        // fixme: who is the owner? physical memory leak!!
        mem_map_page(vaddr + i * 0x1000, obj->physical_pages[i], flags);
    }

    return vaddr;
}

void sharedmem_unmap(s32 id, void* vaddr) {
    assert(sharedmem_exists(id));
    SharedMemory* obj = &shmem[id];

    for (int i = 0; i < obj->size_in_pages; i++) {
        mem_unmap_page(((u32) vaddr) + 0x1000 * i);
    }

    if (vaddr >= 0xC0000000) {
        // mapped to kernel space
        mark_pages_unused_in_bitmap(kernel_sharedmem_bitmap, (((u32) vaddr) - KERNEL_SHARED_MEMORY) / 0x1000, obj->size_in_pages);
    }
}

static void mark_pages_used_in_bitmap(u8* bitmap, u32 start, u32 num) {
    // todo: fix this shit
    u32 index = 0;

	for (u32 b = 0; b < 8192; b++) {
		for (u32 i = 0; i < 8; i++) {
            if (index >= start)
                bitmap[b] |= 1 << i;

            index++;
            if (index >= start + num)
                return;
        }
    }
}

static void mark_pages_unused_in_bitmap(u8* bitmap, u32 start, u32 num) {
    // todo: fix this shit
    u32 index = 0;

	for (u32 b = 0; b < 8192; b++) {
		for (u32 i = 0; i < 8; i++) {
            if (index >= start)
                bitmap[b] &= ~(1 << i);

            index++;
            if (index >= start + num)
                return;
        }
    }
}

static u32 find_available_virtual_region(u32 num_pages, bool map_to_kernel) {
    u32 search_start = map_to_kernel ? KERNEL_SHARED_MEMORY : USER_SHARED_MEMORY;
    u8* bitmap = map_to_kernel ? kernel_sharedmem_bitmap : current_task->sharedmem_bitmap;

    u32 index = 0;
    s32 num = 0;

	for (u32 b = 0; b < 8192; b++) {
		u8 byte = bitmap[b];
		// if (byte == 0xFF) {
        //     num = 0;
		// 	continue;
        // }

		for (u32 i = 0; i < 8; i++) {
			bool used = byte >> i & 1;
            
            if (!used) {
                num++;
                if (num == num_pages) {
                    u32 start = index - num_pages + 1;
                    mark_pages_used_in_bitmap(bitmap, start, num_pages);
                    return search_start + start * 0x1000;
                }
            } else {
                num = 0;
            }

            index++;
		}
	}

    assert(0);
    return 0;

    // todo

    // u32* pdir = REC_PAGEDIR;
    // u32 start = 0;
    // u32 end = 0;
    // u32 available_pages;
    // for (int p = USER_SHARED_MEMORY / 0x1000; p < KERNEL_START / 0x1000;) {
    //     if (!(pdir[p / 1024] & PAGE_FLAG_PRESENT)) {
    //         p += 1024;
    //     } else {
    //         // p
    //     }
    // }
}

static s32 find_available_shmem_slot() {
    for (int i = 0; i < MAX_SHAREDMEM_OBJS; i++) {
        if (!sharedmem_exists(i)) {
            return i;
        }
    }
    assert_msg(0, "no more shmem slots!");
}

void* sharedmem_map_syscall(s32 id) {
    // kernel_log("sharedmem_map_syscall");
    return sharedmem_map(id, false);
}
