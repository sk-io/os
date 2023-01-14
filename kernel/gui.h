#pragma once

#include "types.h"

extern s32 gui_width, gui_height;
extern s32 cursor_x, cursor_y;
extern s32 prev_cursor_x, prev_cursor_y;
extern bool gui_needs_redraw;
extern bool gui_left_click;
extern u16 fake_console_buffer[50 * 80];

void init_gui(s32 width, s32 height);
void gui_thread_entry();
void draw_debug_console(u32 color);
