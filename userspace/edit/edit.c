#include <os.h>
#include <types.h>
#include <sgfx.h>
#include <stdio.h>
#include <string.h>

#define width 360
#define height 240

#define textbuffer_width (width/10)
#define textbuffer_height (height/10)

struct {
    char text[textbuffer_width * textbuffer_height];
    int cursor_x;
    int cursor_y;
    const char* path;
} edit;

GraphicsContext ctx;

static void draw_buffer() {
    int i = 0;
    for (int y = 0; y < textbuffer_height; y++) {
        for (int x = 0; x < textbuffer_width; x++) {
            char c = edit.text[i++];
            if (c == 0)
                continue;
            
            sgfx_draw_char(&ctx, c, x * 10, y * 10 + 2, 0xFFFFFFFF);
        }
    }
}

static void read_file() {
    u32 fp = os_open_file(edit.path);
    u32 size = os_get_file_size(fp);
    if (size >= textbuffer_width * textbuffer_height)
        size = textbuffer_width * textbuffer_height;
    os_read_file(fp, edit.text, size);
    os_close_file(fp);
}

static void save_file() {
    u32 fp = os_open_file(edit.path);
    os_write_file(fp, edit.text, edit.cursor_x);
    os_close_file(fp);
}

static void handle_key_event(const OSKeyboardEvent* event) {
    // printf("key: %u %x\n", event->ascii, event->flags);

    if (!event->state)
        return;

    if (event->ascii == 's' && event->flags & OS_CTRL_HELD) {
        if (edit.path == NULL)
            return;
        printf("saving %s\n", edit.path);
        save_file();
        return;
    }

    if (event->ascii != 0)
        edit.text[edit.cursor_x++] = (char) event->ascii;
}

int main(int argc, char* argv[]) {
    memset(&edit, 0, sizeof(edit));

    if (argc > 1) {
        printf("opening file: %s\n", argv[1]);
        edit.path = argv[1];
        read_file();
    }

    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "Text Editor");
    int* fb = os_map_window_framebuffer(window);

    sgfx_init(&ctx, fb, width, height);

    OSEvent event;

    while (1) {
        sgfx_fill(&ctx, 0xFF000000);
        draw_buffer();

        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_KEYBOARD) {
                OSKeyboardEvent* e = (OSKeyboardEvent*) &event;

                handle_key_event(e);
            }
        }
    }

    return 0;
}
