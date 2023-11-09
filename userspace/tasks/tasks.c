#include <os.h>
#include <types.h>
#include <string.h>
#include <sgfx.h>
#include <stdio.h>

#define width 240
#define height 320

#define MAX_ENTRIES 64
#define FILE_LIST_Y_OFFSET 32
#define FILE_LIST_Y_SPACING 12

OSTaskInfo entries[MAX_ENTRIES] = {0};
int num_entries = 0;
int selected_entry = -1;

static void handle_mouse_move(int x, int y);
static void handle_left_click();
static void update_task_info();

static GraphicsContext ctx;

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "Task Manager");
    int* fb = os_map_window_framebuffer(window);

    sgfx_init(&ctx, fb, width, height);

    unsigned int prev_left_mouse_state = 0;
    u32 last_mouse_button_state = 0;

    update_task_info();

    os_set_timer_interval(0, 500);

    OSEvent event;
    while (1) {
        sgfx_fill(&ctx, 0xFF000000);
        sgfx_draw_string(&ctx, "Tasks:", 3, 5, 0xFFFFFFFF);

        for (int i = 0; i < num_entries; i++) {
            const OSTaskInfo* entry = &entries[i];
            char buffer[128];
            snprintf(buffer, sizeof(buffer), "task id=%u state=%u", entry->id, entry->state);
            
            sgfx_draw_string(&ctx, buffer, 3, FILE_LIST_Y_OFFSET + FILE_LIST_Y_SPACING * i, 0xFFFFFFFF);
        }

        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_MOUSE_MOVE || event.type == OS_EVENT_MOUSE_CLICK) {
                OSMouseEvent* e = (OSMouseEvent*) &event;

                handle_mouse_move(e->x, e->y);

                if (e->buttons) {
                    handle_left_click(e->x, e->y);
                }

                last_mouse_button_state = e->buttons;
            }

            if (event.type == OS_EVENT_TIMER) {
                OSTimerEvent* e = (OSTimerEvent*) &event;

                if (e->timer_id == 0)
                    update_task_info();
            }
        }
    }
    return 0;
}

static void handle_mouse_move(int x, int y) {
    selected_entry = -1;

    if (x < 3 || x > width - 3)
        return;
    
    if (y < FILE_LIST_Y_OFFSET || y >= FILE_LIST_Y_OFFSET + num_entries * FILE_LIST_Y_SPACING)
        return;
    
    selected_entry = (y - FILE_LIST_Y_OFFSET) / FILE_LIST_Y_SPACING;
}

static void handle_left_click(int x, int y) {
    if (selected_entry == -1 || selected_entry >= num_entries)
        return;
}

static void update_task_info() {
    os_get_task_info(entries, MAX_ENTRIES, &num_entries);
}
