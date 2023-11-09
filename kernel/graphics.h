#pragma once

// basic software graphics

#include "types.h"
#include <sgfx.h>

extern GraphicsContext graphics;

void init_graphics(u32* framebuffer, u32 width, u32 height, u32 bpp, u32 pitch);
void graphics_fill(u32 color);
void graphics_fill_rect(s32 x, s32 y, s32 width, s32 height, u32 color);
void graphics_copy_rect(s32 xdest, s32 ydest, s32 width, s32 height, s32 xsrc, s32 ysrc, u32* source);
void graphics_copy_backbuffer();
void graphics_draw_char(u8 c, s32 x0, s32 y0, u32 color);
void graphics_draw_string(const char* str, s32 x0, s32 y0, u32 color);
void graphics_draw_hline(s32 x, s32 y, s32 w, u32 color);
void graphics_draw_vline(s32 x, s32 y, s32 h, u32 color);
