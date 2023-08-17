#pragma once

// GUI window management

#include "types.h"

#define WINDOW_CONTENT_XOFFSET 1
#define WINDOW_CONTENT_YOFFSET 20

#define CLOSE_BUTTON_WIDTH 18
#define CLOSE_BUTTON_HEIGHT 18

#define WINDOW_TITLE_MAX_LENGTH 64
#define WINDOW_FLAG_FULLSCREEN (1 << 0)
#define WINDOW_FLAG_DOUBLE_BUFFERED (1 << 1)

typedef struct {
    u16 state;
    s16 x; // top left of actual window
    s16 y;
    s16 width; // content/inner width
    s16 height; // content/inner height
    u32 flags;
    s16 actual_width;
    s16 actual_height;
    u32* framebuffer;
    u32 framebuffer_size_bytes;
    s16 fb_shmem_id;
    u8 shown_buffer; // buffer being displayed. 0 = first half, 1 = second half (starting at width * height * 4)
    char title[WINDOW_TITLE_MAX_LENGTH];
    s32 owner_task_id;
} Window;

#define MAX_WINDOWS 64
extern Window windows[MAX_WINDOWS];

extern s32 currently_dragging_window;
extern s32 focused_window;
extern s32 window_under_cursor;
extern bool window_under_cursor_inside_content;

void init_windows();
s32 create_window(s32 width, s32 height, u32 flags);
void destroy_window(s32 window_id);
bool check_window_close(s32 window, s32 x, s32 y);
s32 find_window_from_pos(s32 x, s32 y, bool* inside_content);
Window* get_window(s32 id);
void move_window_to_front(s32 window);
