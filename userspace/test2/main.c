#include <os.h>
#include <types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define width 250
#define height 250

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "test2");

    int* fb = os_map_window_framebuffer(window);
    bool state = 0;

    for (int i = 0; i < width * height; i++)
        fb[i] = 0xFFFF0000;
    
    os_set_timer_interval(0, 1000);

    while (1) {
        os_wait_for_events();
        OSEvent event;
        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_TIMER) {
                state = !state;
                for (int i = 0; i < width * height; i++)
                    fb[i] = 0xFF000000 | (state ? 0xFF : 0);

                os_swap_window_buffers(window);
            }
        }
    }

    return 0;
}
