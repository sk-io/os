#include "graphics.h"

#include "memory.h"
#include "kmalloc.h"
#include "util.h"

GraphicsState graphics = {0};

void init_graphics(u32* framebuffer_paddr, u32 width, u32 height, u32 bpp, u32 pitch) {
    graphics.framebuffer = (u32*) KERNEL_GFX;
    graphics.width = width;
    graphics.height = height;
    graphics.bpp = bpp;
    graphics.pitch = pitch;
    graphics.backbuffer = (u32*) kmalloc(width * height * bpp);
    graphics.bytesize = width * height * bpp;

    u32 size_bytes = width * height * bpp;
    u32 needed_page_count = size_bytes / 0x1000 + 1;

    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_GFX + offset), ((u32) framebuffer_paddr) + offset, 0);
    }
}

void graphics_fill(u32 color) {
    u32 size = graphics.bytesize >> 2;
    for (u32 i = 0; i < size; i++) {
        graphics.backbuffer[i] = color;
    }
}

void graphics_fill_rect(s32 x, s32 y, s32 width, s32 height, u32 color) {
    // TODO: optimize?
    for (s32 yi = 0; yi < height; yi++) {
        s32 _y = yi + y;
        if (_y < 0 || _y >= (s32) graphics.height)
            continue;

        for (s32 xi = 0; xi < width; xi++) {
            s32 _x = xi + x;
            if (_x < 0 || _x >= (s32) graphics.width)
                continue;
            
            graphics.backbuffer[_x + _y * graphics.width] = color;
        }
    }
}

void graphics_copy_rect(s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source) {
    for (s32 yi = 0; yi < height; yi++) {
        s32 _y = yi + ydest;
        if (_y < 0 || _y >= (s32) graphics.height)
            continue;

        for (s32 xi = 0; xi < width; xi++) {
            s32 _x = xi + xdest;
            if (_x < 0 || _x >= (s32) graphics.width)
                continue;
            
            u32 c = source[(xsrc + xi) + (ysrc + yi) * width];
            if ((c >> 24) == 0)
                continue;
            graphics.backbuffer[_x + _y * graphics.width] = c;
        }
    }
}

void graphics_copy_backbuffer() {
    int total = graphics.width * graphics.height;
    for (int i = 0; i < total; i++) {
        graphics.framebuffer[i] = graphics.backbuffer[i];
    }
    // memcpy((u8*) graphics.framebuffer, (const u8*) graphics.backbuffer, graphics.width * graphics.height * graphics.bpp);
}

#include "res/font.h"
#define READ_BIT(bitmap, index) (bitmap[((u32) index) / 8] & (0x80 >> (((u32) index) % 8)))

void graphics_draw_char(u8 c, s32 x0, s32 y0, u32 color) {
    s32 char_x = (s32) (c & 0xF) * 10;
    s32 char_y = (s32) (c >> 4) * 10;

    for (int y = 0; y < 10; y++) {
        int sy = y0 + y;
        if (sy < 0 || sy >= (s32) graphics.height)
            continue;

        for (int x = 0; x < 10; x++) {
            int sx = x0 + x;
            if (sx < 0 || sx >= (s32) graphics.width)
                continue;

            int iy = y + char_y;
            int ix = x + char_x;
            u8 b = READ_BIT(font_bitmap, (ix + iy * 160));

            if (!b)
                continue;
            
            graphics.backbuffer[sx + sy * graphics.width] = color;
        }
    }
}

void graphics_draw_string(const char* str, s32 x0, s32 y0, u32 color) {
    char* c = str;

    s32 x = x0;
    s32 y = y0;
    while (*c != '\0') {
        if (*c != ' ')
            graphics_draw_char(*c, x, y, color);

        c++;
        x += 10;
    }
}

void graphics_draw_hline(s32 x, s32 y, s32 w, u32 color) {
    if (w <= 0)
        return;
    if (y < 0 || y >= graphics.height)
        return;
    
    s32 x0 = x;
    s32 x1 = x + w;

    x0 = CLAMP(x0, 0, graphics.width);
    x1 = CLAMP(x1, 0, graphics.width);

    if (x1 - x0 <= 0)
        return;

    s32 i = x0 + y * graphics.width;
    s32 d = i + x1 - x0;

    while (i < d)
        graphics.backbuffer[i++] = color;
}

void graphics_draw_vline(s32 x, s32 y, s32 h, u32 color) {
    if (h <= 0)
        return;
    if (x < 0 || x >= graphics.width)
        return;
    
    s32 y0 = y;
    s32 y1 = y + h;

    y0 = CLAMP(y0, 0, graphics.height);
    y1 = CLAMP(y1, 0, graphics.height);

    if (y1 - y0 <= 0)
        return;

    s32 i = x + y0 * graphics.width;
    s32 d = i + (y1 - y0) * graphics.width;

    while (i < d) {
        graphics.backbuffer[i] = color;
        i += graphics.width;
    }
}
