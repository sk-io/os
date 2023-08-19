#include "kmalloc.h"

#include "util.h"
#include "memory.h"
#include "physalloc.h"
#include "log.h"

#define ALIGNMENT 16

typedef struct {
	u32 size : 31; // including this header
	u32 used : 1;
#if ALIGNMENT > 4
	u32 pad[(ALIGNMENT / 4) - 1];
#endif
} ChunkHeader;

// last chunk has to be 0 to know when to stop

static u32 heap_start;
static u32 heap_size;
static u32 threshold;
static bool kmalloc_initialized = false;

void merge_free_chunks();
void change_heap_size(int new_size);

void kmalloc_init(u32 initial_heap_size) {
	heap_start = KERNEL_MALLOC;
	heap_size = 0;
    threshold = 0;
	kmalloc_initialized = true;

    change_heap_size(initial_heap_size);

	//memset(pool_start, 0, pool_size);
	*((u32*)heap_start) = 0;
}

void* kmalloc(u32 bytes) {
	assert_msg(kmalloc_initialized, "not initialized!");
    assert_msg(bytes, "tried to kmalloc 0 bytes!");

	u32 real_size = bytes + sizeof(ChunkHeader);
	if (real_size & (ALIGNMENT - 1)) {
		real_size += ALIGNMENT - (real_size & (ALIGNMENT - 1));
	}

	u32 pos = heap_start;
	while (true) {
		if (pos + real_size + sizeof(ChunkHeader) > heap_start + heap_size) {
            // download more ram
			// the +sizeof(ChunkHeader) is for the last stop chunk
            change_heap_size(pos + real_size + sizeof(ChunkHeader) - heap_start); // +1 ?
		}

		ChunkHeader* chunk = (ChunkHeader*) pos;

		if (!chunk->used && (chunk->size >= real_size || chunk->size == 0)) {
			// use this
			chunk->used = 1;

			if (chunk->size == 0) {
				// last chunk
				chunk->size = real_size;

				// make sure last chunk is 0
				ChunkHeader* last = (ChunkHeader*)(pos + real_size);
				last->size = 0;
				last->used = 0;
			} else {
				s32 diff = (s32)chunk->size - (s32)real_size;

				if (diff >= 8) { // big enough for 1 4byte malloc
					// if big enough, split empty space into separate
					ChunkHeader* empty = (ChunkHeader*)(pos + real_size);
					empty->size = diff;
					empty->used = 0;

					chunk->size = real_size;
				} else {
					// use up the whole thing
				}
			}

			u32 addr = pos + sizeof(ChunkHeader);
			if (addr > threshold)
				threshold = addr;
			return (void*) addr;
		}

		pos += chunk->size;
	}

    assert_msg(false, "something wen't wrong in kmalloc");
	return 0;
}

void kfree(void* addr) {
	assert_msg(kmalloc_initialized, "not initialized!");
	ChunkHeader* chunk = (ChunkHeader*) (((u32) addr) - sizeof(ChunkHeader));

	if (!chunk->used) {
		return;
	}
	chunk->used = 0;

	// check if any/both adjacent chunks are empty
	//     !! cant do this, no way of going backwards
	// guess we'll just have to loop through the whole thing..
	merge_free_chunks();
}

void merge_free_chunks() {
	u32 pos = heap_start;
	while (true) {
		ChunkHeader* chunk = (ChunkHeader*)pos;
		if (chunk->size == 0)
			break;

		if (!chunk->used) {
			u32 next_pos = ((u32)chunk) + chunk->size;
			while (true) {
				ChunkHeader* next = (ChunkHeader*) next_pos;
				// swallow it.
				if (next->used)
					break;

				if (next->size == 0) {
					// we've reached the end, stop merging
					chunk->size = 0;
					threshold = (u32)chunk;
					return;
				}
				chunk->size += next->size;
				next_pos += next->size;
				//next->size = 69; // debug to show swallowed chunks, can't do this if we want to fill with debug
			}
		}

		pos += chunk->size;
		if (pos >= heap_start + heap_size) {
			break;
		}
	}
}

u32 kmalloc_get_total_bytes() {
	return threshold - heap_start;
}

void change_heap_size(int new_size) {
    int old_page_top = CEIL_DIV(heap_size, 0x1000);
    int new_page_top = CEIL_DIV(new_size, 0x1000);
    if (new_page_top > old_page_top) {
        int num = new_page_top - old_page_top;

        // kernel_log("expanding kernel heap by %d pages", num);

        for (int i = 0; i < num; i++) {
            u32 phys = pmm_alloc_pageframe();
            mem_map_page(KERNEL_MALLOC + old_page_top * 0x1000 + i * 0x1000, phys, PAGE_FLAG_WRITE);
        }
    } else if (new_page_top < old_page_top) {
        // todo
    }

    heap_size = new_size;
}
