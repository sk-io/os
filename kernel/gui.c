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

#define GUI_DRAW_INTERVAL 30

GUI gui;

struct {
    s32 x, y;
    s32 w, h;
} file_button;

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
    gui.needs_update = true;
    
    init_windows();
    
    file_button.w = 130;
    file_button.h = 20;
    file_button.x = graphics.width - file_button.w - 10;
    file_button.y = graphics.height - file_button.h - 10;
}

void gui_thread_entry() {
    // disable_interrupts();

    while (1) {
        while (!should_gui_redraw());

        push_cli();
        gui_handle_events();
        gui_draw_frame();
        gui.needs_update = false;
        gui.last_redraw_time = get_system_time_millis();
        pop_cli();
    }
}

static void gui_draw_frame() {
    // clear screen
    sgfx_fill(&graphics, 0xFFFFFFFF);

    // background terminal thing
    draw_debug_console(0);

    // file button
    sgfx_draw_box(&graphics, file_button.x, file_button.y, file_button.w, file_button.h, 0x8ecae6, 0);
    sgfx_draw_string(&graphics, "File Browser", file_button.x + 5, file_button.y + 5, 0);

    draw_windows();

    // redraw indicator
    sgfx_fill_rect(&graphics, graphics.width - 10, 2, 8, 8, redraw_indicator ? 0xFF00FF : 0);
    redraw_indicator ^= 1;

    // draw time
    u64 time = get_system_time_millis();
    // if (time)
    //     time /= 1000;
    char status_str[128];

    int phys_mem = pmm_get_total_allocated_pages() * 4;
    snprintf(status_str, sizeof(status_str), "phys used: %dKiB   systime: %u", phys_mem, time);
    sgfx_draw_string(&graphics, status_str, 3, graphics.height - 15, 0);

    sgfx_copy_rect_alpha(&graphics, gui.cursor_x, gui.cursor_y, 12, 19, 0, 0, res_cursor_raw);

    graphics_copy_backbuffer();
}

static void handle_left_click() {
    if (gui.cursor_x >= file_button.x && gui.cursor_x < file_button.x + file_button.w
        && gui.cursor_y >= file_button.y && gui.cursor_y < file_button.y + file_button.h) {
        create_user_task("/files.exe", NULL);
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

    gui.needs_update = true;
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
        gui.needs_update = true;

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
            sgfx_draw_char(&graphics, c & 0xFF, x * 10, y * 10, color);
            index++;
        }
    }
}

bool should_gui_redraw() {
    return gui.needs_update && (get_system_time_millis() - gui.last_redraw_time) >= GUI_DRAW_INTERVAL;
}
