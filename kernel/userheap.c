#include "userheap.h"

#include "util.h"
#include "memory.h"
#include "physalloc.h"
#include "log.h"
#include "tasks.h"

void init_user_heap(Task* task) {
	task->heap_start = task->heap_end = USER_HEAP_START;
}

void set_user_heap_end(Task* task, u32 new_heap_end) {
	assert(new_heap_end > task->heap_start);
    // kernel_log("heap start is %x", task->heap_start);

    int old_page_top = CEIL_DIV(task->heap_end, 0x1000);
    int new_page_top = CEIL_DIV(new_heap_end, 0x1000);

    if (new_page_top > old_page_top) {
        int num = new_page_top - old_page_top;

        kernel_log("expanding process heap by %d pages", num);

        for (int i = 0; i < num; i++) {
            u32 phys = pmm_alloc_pageframe();
            u32 virt = old_page_top * 0x1000 + i * 0x1000;

            // kernel_log("mapping %x", virt);

            mem_map_page(virt, phys, PAGE_FLAG_WRITE | PAGE_FLAG_USER | PAGE_FLAG_OWNER);

            // zero memory to prevent leaking data
            memset((void*) virt, 0, 0x1000);
        }
    } else if (new_page_top < old_page_top) {
        // todo
		kernel_log("userheap.c tried to shrink the heap!!");
    }

    task->heap_end = new_heap_end;
}
