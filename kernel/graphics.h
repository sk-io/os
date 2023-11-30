#pragma once

// basic software graphics

#include "types.h"
#include <sgfx.h>

extern GraphicsContext graphics;

void init_graphics(u32* framebuffer, u32 width, u32 height, u32 bpp, u32 pitch);
void graphics_copy_backbuffer();
