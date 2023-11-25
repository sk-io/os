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
#include "time.h"
#include "interrupts.h"

GUI gui;

s32 testbutton_x, testbutton_y;
s32 testbutton_w, testbutton_h;

s32 redraw_indicator = 0;

static void gui_handle_events();
static void gui_draw_frame();

void init_gui(s32 width, s32 height) {
    gui.width = width;
    gui.height = height;
    gui.cursor_x = width / 2;
    gui.cursor_y = height / 2;
    gui.prev_cursor_x = gui.cursor_x;
    gui.prev_cursor_y = gui.cursor_y;
    gui.needs_redraw = true;

    init_windows();
    
    testbutton_x = graphics.width - 100;
    testbutton_y = graphics.height - 100;
    testbutton_w = 50;
    testbutton_h = 50;
}

void gui_thread_entry() {
    // disable_interrupts();

    while (1) {
        if (gui.needs_redraw) {
            gui.needs_redraw = false;
            gui_draw_frame();
        }

        push_cli();
        gui_handle_events();
        pop_cli();
    }
}

static void gui_draw_frame() {
    graphics_fill(0xFFFFFFFF);

    draw_debug_console(0);
    draw_windows();

    graphics_fill_rect(testbutton_x, testbutton_y, testbutton_w, testbutton_h, 0xFFFF00FF);

    graphics_fill_rect(graphics.width - 10, 2, 8, 8, redraw_indicator ? 0xFF00FF : 0);
    redraw_indicator ^= 1;

    // draw time
    u64 time = get_system_time_millis();
    // if (time)
    //     time /= 1000;
    char time_str[128];

    int phys_mem = pmm_get_total_allocated_pages() * 4;

    sprintf(time_str, "phys used: %dKiB   systime: %u", phys_mem, time);
    graphics_draw_string(time_str, 3, graphics.height - 15, 0);

    graphics_copy_rect(gui.cursor_x, gui.cursor_y, 12, 19, 0, 0, res_cursor_raw);

    graphics_copy_backbuffer();
}

static void handle_left_click() {
    if (gui.cursor_x >= testbutton_x && gui.cursor_x < testbutton_x + testbutton_w
        && gui.cursor_y >= testbutton_y && gui.cursor_y < testbutton_y + testbutton_h) {
        create_user_task("files.exe");
    }

    focused_window = window_under_cursor;

    if (window_under_cursor != -1) {
        // we are clicking on a window
        move_window_to_front(window_under_cursor);

        if (window_under_cursor_inside_content) {
            Event click;
            click.type = EVENT_MOUSE_CLICK;
            click.data0 = gui.cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            click.data1 = gui.cursor_y - windows[window_under_cursor].y - WINDOW_CONTENT_YOFFSET;
            click.data2 = 1;
            handle_event(&click);
        } else {
            // we are clicking on its border
            if (check_window_close(window_under_cursor, gui.cursor_x, gui.cursor_y)) {
                Window* w = get_window(window_under_cursor);
                u32 task_id = w->owner_task_id;
                // todo: allow for multiple windows
                destroy_window(window_under_cursor);
                kill_task(task_id);
                window_under_cursor = -1;
                focused_window = -1;
            }
            currently_dragging_window = window_under_cursor;
        }
    }

    gui.needs_redraw = true;
}

static void gui_handle_events() {
    gui.cursor_x += mouse.x_acc;
    mouse.x_acc = 0;
    gui.cursor_y += mouse.y_acc;
    mouse.y_acc = 0;

    if (gui.cursor_x < 0)
        gui.cursor_x = 0;
    if (gui.cursor_y < 0)
        gui.cursor_y = 0;
    if (gui.cursor_x >= graphics.width)
        gui.cursor_x = graphics.width - 1;
    if (gui.cursor_y >= graphics.height)
        gui.cursor_y = graphics.height - 1;

    window_under_cursor = find_window_from_pos(gui.cursor_x, gui.cursor_y, &window_under_cursor_inside_content);

    static bool prev_mouse_left_button = 0;

    if (prev_mouse_left_button != gui.left_click) {
        if (gui.left_click) {
            handle_left_click();
        } else {
            currently_dragging_window = -1;
        }

        prev_mouse_left_button = gui.left_click;
    }
    
    gui.left_click = mouse.left_button;

    s32 dx = gui.cursor_x - gui.prev_cursor_x;
    s32 dy = gui.cursor_y - gui.prev_cursor_y;

    if (dx != 0 || dy != 0) {
        gui.needs_redraw = true;

        if (currently_dragging_window != -1) {
            windows[currently_dragging_window].x += dx;
            windows[currently_dragging_window].y += dy;
        } else if (window_under_cursor_inside_content) { // && focused_window == window_under_cursor
            Event move;
            move.type = EVENT_MOUSE_MOVE;
            move.data0 = gui.cursor_x - windows[window_under_cursor].x - WINDOW_CONTENT_XOFFSET;
            move.data1 = gui.cursor_y - windows[window_under_cursor].y - WINDOW_CONTENT_YOFFSET;
            move.data2 = 0;
            handle_event(&move);
        }

        gui.prev_cursor_x = gui.cursor_x;
        gui.prev_cursor_y = gui.cursor_y;
    }
}

void draw_debug_console(u32 color) {
    int index = 0;
    for (int y = 0; y < 50; y++) {
        for (int x = 0; x < 80; x++) {
            u16 c = gui.fake_console_buffer[index];
            graphics_draw_char(c & 0xFF, x * 10, y * 10, color);
            index++;
        }
    }
}
