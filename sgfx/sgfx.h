#pragma once

#include "../kernel/types.h"

// basic software graphics library

typedef struct {
    u32* framebuffer;
    u32 width;
    u32 height;
    u32 bytesize;
} GraphicsContext;

void sgfx_init(GraphicsContext* ctx, u32* framebuffer, u32 width, u32 height);
void sgfx_fill(const GraphicsContext* ctx, u32 color);
void sgfx_fill_rect(const GraphicsContext* ctx, s32 x, s32 y, s32 width, s32 height, u32 color);
void sgfx_copy_rect(const GraphicsContext* ctx, s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source);
void sgfx_draw_char(const GraphicsContext* ctx, u8 c, s32 x0, s32 y0, u32 color);
void sgfx_draw_string(const GraphicsContext* ctx, const char* str, s32 x0, s32 y0, u32 color);
void sgfx_draw_hline(const GraphicsContext* ctx, s32 x, s32 y, s32 w, u32 color);
void sgfx_draw_vline(const GraphicsContext* ctx, s32 x, s32 y, s32 h, u32 color);
void sgfx_pixel(GraphicsContext* context, s32 x, s32 y, u32 color);
void sgfx_draw_line(GraphicsContext* context, s32 x0, s32 y0, s32 x1, s32 y1, u32 color);

