#include <os.h>
#include <types.h>

#include "graphics.h"

#define width 240
#define height 360

#define MAX_ENTRIES 64
#define FILE_LIST_Y_OFFSET 20
#define FILE_LIST_Y_SPACING 12

OSFileInfo entries[MAX_ENTRIES] = {0};
int num_entries = 0;
int selected_entry = -1;

char path[256] = {0};

static void handle_mouse_move(int x, int y);
static void handle_left_click();

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "File Browser");
    int* fb = os_map_window_framebuffer(window);

    init_graphics(fb, width, height);

    os_printf("reading dir...");

    if (!os_open_dir("/")) {
        os_printf("failed to open /");
        return 1;
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (!os_next_file_in_dir(&entries[i]))
            break;
        num_entries++;
    }
    
    os_close_dir();

    os_printf("done! read %d entries.", num_entries);

    unsigned int prev_left_mouse_state = 0;

    OSEvent event;
    while (1) {
        graphics_fill(0xFF000000);
        graphics_draw_string("FILES", 3, 5, 0xFFFFFFFF);

        for (int i = 0; i < num_entries; i++) {
            const OSFileInfo* entry = &entries[i];
            
            u32 color = (entry->attributes & OS_FILE_INFO_IS_DIR) ? 0xFF5b6ee1 : 0xFFcbdbfc;
            if (selected_entry == i)
                color = 0xFFFFFFFF;
            graphics_draw_string(entry->name, 3, FILE_LIST_Y_OFFSET + FILE_LIST_Y_SPACING * i, color);
        }

        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) {
            if (event.type == EVENT_MOUSE_MOVE) {
                OSMouseEvent* e = (OSMouseEvent*) &event;

                handle_mouse_move(e->x, e->y);
            }

            if (event.type == EVENT_MOUSE) {
                OSMouseEvent* e = (OSMouseEvent*) &event;
                if (e->buttons) {
                    handle_left_click();
                }
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

static void handle_left_click() {
    if (selected_entry == -1 || selected_entry >= num_entries)
        return;
    
    if (entries[selected_entry].attributes & OS_FILE_INFO_IS_DIR) {
        // change directory
    } else {
        // execute binary
        os_printf("executing");

        os_exec(entries[selected_entry].name);
    }
}
