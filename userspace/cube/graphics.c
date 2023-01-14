#include "graphics.h"

#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b) ? (b) : (x)))
#define abs(x) ((x)<0 ? -(x) : (x))

static void pixel(GraphicsContext* context, s32 x, s32 y, u32 color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= context->width) return;
    if (y >= context->height) return;
    context->framebuffer[x + y * context->width] = color;
}

void init_graphics(GraphicsContext* context, u32* framebuffer, u32 width, u32 height) {
    context->framebuffer = framebuffer;
    context->width = width;
    context->height = height;
    context->bytesize = width * height * 4;
}

void graphics_fill(GraphicsContext* context, u32 color) {
    u32 size = context->bytesize >> 2;
    for (u32 i = 0; i < size; i++) {
        context->framebuffer[i] = color;
    }
}

void graphics_line(GraphicsContext* context, s32 x0, s32 y0, s32 x1, s32 y1, u32 color) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1; 
    int err = dx + dy, e2;

    for (int i = 0; i < 1000; i++) {
        pixel(context, x0, y0, color);

        if (x0 == x1 && y0 == y1)
            break;
        
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}
