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
    u32 needed_page_count = size_bytes / 0x1000 + 1;

    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((u32) framebuffer_paddr) + offset, 0);
    }
}

// TODO: get rid of these and just use sgfx_ instead

void graphics_fill(u32 color) {
    sgfx_fill(&graphics, color);
}

void graphics_fill_rect(s32 x, s32 y, s32 width, s32 height, u32 color) {
    sgfx_fill_rect(&graphics, x, y, width, height, color);
}

void graphics_copy_rect(s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source) {
    sgfx_copy_rect(&graphics, xdest, ydest, width, height, xsrc, ysrc, source);
}

void graphics_copy_backbuffer() {
    int total = graphics.width * graphics.height;
    for (int i = 0; i < total; i++) {
        frontbuffer[i] = graphics.framebuffer[i];
    }
}

void graphics_draw_char(u8 c, s32 x0, s32 y0, u32 color) {
    sgfx_draw_char(&graphics, c, x0, y0, color);
}

void graphics_draw_string(const char* str, s32 x0, s32 y0, u32 color) {
    sgfx_draw_string(&graphics, str, x0, y0, color);
}

void graphics_draw_hline(s32 x, s32 y, s32 w, u32 color) {
    sgfx_draw_hline(&graphics, x, y, w, color);
}

void graphics_draw_vline(s32 x, s32 y, s32 h, u32 color) {
    sgfx_draw_vline(&graphics, x, y, h, color);
}
