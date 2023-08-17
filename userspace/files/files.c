#include <os.h>
#include <types.h>

#include "graphics.h"

#define width 240
#define height 360

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "File Browser");
    int* fb = os_map_window_framebuffer(window);

    init_graphics(fb, width, height);

    os_printf("opening dir...");

    if (!os_open_dir("/")) {
        os_printf("failed to open /");
        return 1;
    }

    os_printf("reading files...");

    char buffer[256];
    while (os_next_file_in_dir(buffer, sizeof(buffer))) {
        os_printf("%s", buffer);
    }

    os_printf("closing dir...");
    os_close_dir();
    
    os_printf("done!");

    OSEvent event;
    while (1) {
        graphics_fill(0xFF000000);
        // int i = 0;
        // for (int y = 0; y < textbuffer_height; y++) {
        //     for (int x = 0; x < textbuffer_width; x++) {
        //         char c = textbuffer[i++];
        //         if (c == 0)
        //             continue;
                
        //         graphics_draw_char(c, x * 10, y * 10 + 2, 0xFFFFFFFF);
        //     }
        // }

        graphics_draw_string("TEST", 3, 5, 0xFFFFFFFF);

        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) {
            // if (event.type == EVENT_KEYBOARD) {
            //     OSKeyboardEvent* e = (OSKeyboardEvent*) &event;

            //     if (e->state)
            //         textbuffer[cursor_x++] = (char) e->ascii;
            // }
        }
    }
    return 0;
}
