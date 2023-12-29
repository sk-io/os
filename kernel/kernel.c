#include "types.h"
#include "log.h"
#include "gdt.h"
#include "interrupts.h"
#include "util.h"
#include "pit.h"
#include "keyboard.h"
#include "console.h"
#include "multiboot.h"
#include "memory.h"
#include "kmalloc.h"
#include "physalloc.h"
#include "tasks.h"
#include "exceptions.h"
#include "syscall.h"
#include "elf.h"
#include "sharedmem.h"
#include "events.h"
#include "graphics.h"
#include "gui.h"
#include "mouse.h"
#include "fpu.h"
#include "slib.h"
#include "disk.h"

bool graphics_enabled;

void kernel_main(struct multiboot_info* info) {
    disable_interrupts();
    setup_gdt();

    graphics_enabled = info->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB;

    init_logging(true);
    kernel_log("Hello!");

    u32 framebuffer_addr = info->framebuffer_addr;
    u32 framebuffer_width = info->framebuffer_width;
    u32 framebuffer_height = info->framebuffer_height;
    u32 framebuffer_bpp = info->framebuffer_bpp;
    u32 framebuffer_pitch = info->framebuffer_pitch;

    kernel_log("Available memory: %uMB", info->mem_upper / 1024);

    kernel_log("Setting up interrupts");
    setup_interrupts();
    init_exceptions();
    init_syscalls();

    //enable_fpu();
    
    init_disks(info);

    init_timer(1000);

    // if not using ramdisk, start after the kernel at 16MB
    u32 phys_alloc_start = 0x100000 * 16;
    if (using_ramdisk) {
        // otherwise, start after the ramdisk
        phys_alloc_start = (ramdisk.phys_addr + ramdisk.size + 0xFFF) & ~0xFFF;
    }
    kernel_log("Starting physical allocation from %x", phys_alloc_start);
    init_memory(info->mem_upper * 1024, phys_alloc_start);

    kernel_log("Initial pagedir is at %x", mem_get_current_page_directory());
    kernel_log("Setting up kernel heap");
    kmalloc_init(0x1000);

    map_ramdisk();

    setup_tasks();
    sharedmem_init();
    init_shared_libs();
    init_events();

    fat32_init_volume(&primary_volume);

    if (graphics_enabled) {
        kernel_log("framebuffer is at %x", framebuffer_addr);

        init_graphics((u32*) framebuffer_addr, framebuffer_width, framebuffer_height, (u32) framebuffer_bpp / 8, framebuffer_pitch);
        init_gui(framebuffer_width, framebuffer_height);
        gui.task_id = create_kernel_task(gui_thread_entry);
        get_task(gui.task_id)->state = TASK_STATE_IDLE;

        sgfx_fill(&graphics, 0xFFFFFFFF);

        // create_user_task("files.exe", NULL);
    }

    init_keyboard();
    init_mouse();

    set_timer_enabled(true);
    
    kernel_log("Init done.");

    console_set_prompt_enabled(true);
    enable_interrupts();

    while (true) {
        halt();
    }
}
