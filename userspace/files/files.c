#include <os.h>
#include <types.h>
#include <string.h>
#include <sgfx.h>

#define width 240
#define height 320

#define MAX_ENTRIES 64
#define FILE_LIST_Y_OFFSET 32
#define FILE_LIST_Y_SPACING 12

OSFileInfo entries[MAX_ENTRIES] = {0};
int num_entries = 0;
int selected_entry = -1;

char path[256] = {0};

static void handle_mouse_move(int x, int y);
static void handle_left_click();
static void read_directory();
static void go_up();

static GraphicsContext ctx;

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "File Browser");
    int* fb = os_map_window_framebuffer(window);

    sgfx_init(&ctx, fb, width, height);

    strcpy(path, "/");
    read_directory();

    unsigned int prev_left_mouse_state = 0;
    u32 last_mouse_button_state = 0;

    OSEvent event;
    while (1) {
        sgfx_fill(&ctx, 0xFF000000);
        sgfx_draw_string(&ctx, "Path:", 3, 5, 0xFFFFFFFF);
        sgfx_draw_string(&ctx, path, 3 + 10*7, 5, 0xFFFFFFFF);
        sgfx_draw_string(&ctx, "Go up", 3, 5+12, 0xFFFFFFFF);

        for (int i = 0; i < num_entries; i++) {
            const OSFileInfo* entry = &entries[i];
            
            u32 color = (entry->attributes & OS_FILE_INFO_IS_DIR) ? 0xFF5b6ee1 : 0xFFcbdbfc;
            if (selected_entry == i)
                color = 0xFFFFFFFF;
            sgfx_draw_string(&ctx, entry->name, 3, FILE_LIST_Y_OFFSET + FILE_LIST_Y_SPACING * i, color);
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
    if (x > 3 && x < width - 3 && y > 17 && y < 27) {
        go_up();
        read_directory();
        return;
    }

    if (selected_entry == -1 || selected_entry >= num_entries)
        return;
    
    if (entries[selected_entry].attributes & OS_FILE_INFO_IS_DIR) {
        // change directory

        int len = strlen(path);
        if (path[len - 1] != '/')
            strcat(path, "/");

        strcat(path, entries[selected_entry].name);

        read_directory();
    } else {
        // execute binary
        os_exec(entries[selected_entry].name);
    }
}

static void read_directory() {
    num_entries = 0;
    // os_printf("reading from %s", path);

    if (!os_open_dir(path)) {
        os_printf("failed to open %s", path);
        return;
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (!os_next_file_in_dir(&entries[i]))
            break;
        num_entries++;
    }
    
    os_close_dir();
}

static void go_up() {
    if (strcmp(path, "/") == 0)
        return;
    
    char* last = strrchr(path, '/');
    if (last == path)
        last[1] = '\0';
    else
        last[0] = '\0';
}
