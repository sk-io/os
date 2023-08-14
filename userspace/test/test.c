#include <os.h>
#include <types.h>

#define width 300
#define height 240

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, OS_DOUBLE_BUFFERED);
    os_set_window_title(window, "HEAP TEST");

    int* fb = os_map_window_framebuffer(window);

    int heap_start = os_get_heap_start();
    int heap_end = os_get_heap_end();
    os_printf("heap_start=%x heap_end=%x", heap_start, heap_end);

    os_printf("allocating...");
    int* test[100];
    for (int i = 0; i < 100; i++)
        test[i] = os_malloc(4);
    
    os_printf("writing...");
    for (int i = 0; i < 100; i++)
        *(test[i]) = i;

    int shown_buffer = 0;
    int count = 0;
    OSEvent event;
    int size = width * height;

    int x = 0;
    int y = 0;

    while (1) {
        while (os_poll_event(&event)) {}

        int* backbuffer = fb + (shown_buffer == 0 ? size : 0);
        
        // clear
        for (int i = 0; i < size; i++) {
            backbuffer[i] = 0xFF000000 | (shown_buffer ? 0xFF : 0);
        }

        for (int i = 0; i < size; i++) {
            backbuffer[i] = 0xFF000000;
        }

        shown_buffer = os_swap_window_buffers(window);
    }
    return 0;
}
