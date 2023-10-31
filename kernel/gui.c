#include "gui.h"

#include "graphics.h"
#include "util.h"
#include "log.h"
#include "sharedmem.h"
#include "syscall.h"
#include "mouse.h"
#include "tasks.h"
#include "events.h"
#include "windows.h"
#include "res/cursor_img.h"

s32 gui_width, gui_height;
s32 cursor_x, cursor_y;
s32 prev_cursor_x, prev_cursor_y;

bool gui_needs_redraw = true;
bool gui_left_click = false;

s32 testbutton_x, testbutton_y;
s32 testbutton_w, testbutton_h;

s32 redraw_indicator = 0;

u16 fake_console_buffer[50 * 80];

static void gui_handle_events();
static void gui_draw_frame();

void init_gui(s32 width, s32 height) {
    gui_width = width;
    gui_height = height;
    cursor_x = width / 2;
    cursor_y = height / 2;
    prev_cursor_x = cursor_x;
    prev_cursor_y = cursor_y;

    init_windows();
    
    testbutton_x = graphics.width - 100;
    testbutton_y = graphics.height - 100;
    testbutton_w = 50;
    testbutton_h = 50;
}

void gui_thread_entry() {
    while (1) {
        gui_handle_events();

        if (gui_needs_redraw) {
            gui_needs_redraw = false;
            gui_draw_frame();
        }

        // task_schedule();
    }
}

static void gui_draw_frame() {
    graphics_fill(0xFFFFFFFF);

    draw_debug_console(0);
    draw_windows();

    graphics_fill_rect(testbutton_x, testbutton_y, testbutton_w, testbutton_h, 0xFFFF00FF);

    graphics_fill_rect(graphics.width - 10, 2, 8, 8, redraw_indicator ? 0xFF00FF : 0);
    redraw_indicator ^= 1;

    graphics_copy_rect(cursor_x, cursor_y, 12, 19, 0, 0, res_cursor_raw);

    graphics_copy_backbuffer();
}

static void handle_left_click() {
    if (cursor_x >= testbutton_x && cursor_x < testbutton_x + testbutton_w
        && cursor_y >= testbutton_y && cursor_y < testbutton_y + testbutton_h) {
        create_user_task("cube.exe");
    }

    focused_window = window_under_cursor;

    if (window_under_cursor != -1) {
        // we are clicking on a window
        move_window_to_front(window_under_cursor);

        if (window_under_cursor_inside_content) {
            Event click;
            click.type = EVENT_MOUSE_CLICK;
            click.data0 = cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            click.data1 = cursor_y - windows[window_under_cursor].y - WINDOW_CONTENT_YOFFSET;
            click.data2 = 1;
            handle_event(&click);
        } else {
            // we are clicking on its border
            if (check_window_close(window_under_cursor, cursor_x, cursor_y)) {
                Window* w = get_window(window_under_cursor);
                // todo: allow for multiple windows
                kill_task(w->owner_task_id);
                destroy_window(window_under_cursor);
                window_under_cursor = -1;
                focused_window = -1;
            }
            currently_dragging_window = window_under_cursor;
        }
    }

    gui_needs_redraw = true;
}

static void gui_handle_events() {
    cursor_x += mouse_x_acc;
    mouse_x_acc = 0;
    cursor_y += mouse_y_acc;
    mouse_y_acc = 0;

    if (cursor_x < 0)
        cursor_x = 0;
    if (cursor_y < 0)
        cursor_y = 0;
    if (cursor_x >= graphics.width)
        cursor_x = graphics.width - 1;
    if (cursor_y >= graphics.height)
        cursor_y = graphics.height - 1;

    window_under_cursor = find_window_from_pos(cursor_x, cursor_y, &window_under_cursor_inside_content);

    static bool prev_mouse_left_button = 0;

    if (prev_mouse_left_button != gui_left_click) {
        if (gui_left_click) {
            handle_left_click();
        } else {
            currently_dragging_window = -1;
        }

        prev_mouse_left_button = gui_left_click;
    }
    
    gui_left_click = mouse_left_button;

    s32 dx = cursor_x - prev_cursor_x;
    s32 dy = cursor_y - prev_cursor_y;

    if (dx != 0 || dy != 0) {
        gui_needs_redraw = true;

        if (currently_dragging_window != -1) {
            windows[currently_dragging_window].x += dx;
            windows[currently_dragging_window].y += dy;
        } else if (window_under_cursor_inside_content) { // && focused_window == window_under_cursor
            Event move;
            move.type = EVENT_MOUSE_MOVE;
            move.data0 = cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            move.data1 = cursor_y - windows[window_under_cursor].y - WINDOW_CONTENT_YOFFSET;
            move.data2 = 0;
            handle_event(&move);
        }

        prev_cursor_x = cursor_x;
        prev_cursor_y = cursor_y;
    }
}

void draw_debug_console(u32 color) {
    int index = 0;
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 80; x++) {
            u16 c = fake_console_buffer[index];
            graphics_draw_char(c & 0xFF, x * 10, y * 10, color);
            index++;
        }
    }
}
