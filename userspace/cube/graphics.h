#pragma once

#include "types.h"

typedef struct {
    u32* framebuffer;
    u32 width;
    u32 height;
    u32 bpp;
    u32 pitch;
    u32 bytesize;
} GraphicsContext;

void init_graphics(GraphicsContext* context, u32* framebuffer, u32 width, u32 height);
void graphics_fill(GraphicsContext* context, u32 color);
void graphics_line(GraphicsContext* context, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);
