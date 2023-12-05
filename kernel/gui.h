#pragma once

#include "types.h"

typedef struct {
    s32 width, height;
    s32 cursor_x, cursor_y;
    s32 prev_cursor_x, prev_cursor_y;
    bool needs_update;
    bool left_click;
    u16 fake_console_buffer[50 * 80];
    u64 last_redraw_time;
    s32 task_id;
} GUI;

extern GUI gui;

void init_gui(s32 width, s32 height);
void gui_thread_entry();
void draw_debug_console(u32 color);
bool should_gui_redraw();
