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
#include "ramdisk.h"
#include "sharedmem.h"
#include "events.h"
#include "graphics.h"
#include "gui.h"
#include "mouse.h"
#include "fpu.h"

bool graphics_enabled;

static void test() {
    kernel_log("opening root dir..");
    DIR dir;
    f_opendir(&dir, "/");

    kernel_log("file list:");
    FILINFO info;
    while (true) {
        f_readdir(&dir, &info);

        if (info.fname[0] == '\0')
            break;
        
        kernel_log("file: %s", info.fname);
    }

    f_closedir(&dir);
}

void kernel_main(struct multiboot_info* info) {
    disable_interrupts();
    setup_gdt();

    graphics_enabled = info->framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB;

    init_logging(true);
    kernel_log("Hello!");

    u32 mod_count = info->mods_count;
    assert_msg(mod_count, "zero modules!");
    u32 mod0 = *(u32*) (info->mods_addr);
    u32 mod1 = *(u32*) (info->mods_addr + 4);

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

    init_timer(1000);
    set_timer_enabled(true);
    
    init_memory(info->mem_upper * 1024);
    kernel_log("Initial pagedir is at %x", mem_get_current_page_directory());
    kernel_log("Setting up kernel heap");
    kmalloc_init(0x1000);

    setup_tasks();
    sharedmem_init();
    init_events();

    u32 size = mod1 - mod0;
    kernel_log("module addr: %x", mod0);
    kernel_log("module size: %x", size);

    init_ramdisk(mod0 + 0xC0000000, size);

    test();

    if (graphics_enabled) {
        init_graphics((u32*) framebuffer_addr, framebuffer_width, framebuffer_height, (u32) framebuffer_bpp / 8, framebuffer_pitch);
        init_gui(framebuffer_width, framebuffer_height);
        create_kernel_task(gui_thread_entry);

        create_user_task("files.exe");
    }

    init_keyboard();
    init_mouse();

    kernel_log("Init done.");

    console_set_prompt_enabled(true);
    enable_interrupts();

    while (true) {
        halt();
    }
}
