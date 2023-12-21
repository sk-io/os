#include "graphics.h"

#include "memory.h"
#include "kmalloc.h"
#include "util.h"

GraphicsContext graphics;

static u32* frontbuffer;

void init_graphics(u32* framebuffer_paddr, u32 width, u32 height, u32 bpp, u32 pitch) {
    u32* backbuffer = (u32*) kmalloc(width * height * bpp);

    sgfx_init(&graphics, backbuffer, width, height);
    frontbuffer = (u32*) KERNEL_GFX;

    u32 size_bytes = width * height * bpp;
    u32 needed_page_count = CEIL_DIV(size_bytes, 0x1000);

    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((u32) framebuffer_paddr) + offset, 0);
    }
}

void graphics_copy_backbuffer() {
    int total = graphics.width * graphics.height;
    for (int i = 0; i < total; i++) {
        frontbuffer[i] = graphics.framebuffer[i];
    }
}
