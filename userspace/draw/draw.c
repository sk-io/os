#include <os.h>
#include <types.h>

#define width 300
#define height 240

u32* memcpy32(u32* dest, const u32* src, u32 len) {
    len >>= 2;
    for (; len != 0; len--) *dest++ = *src++;
    return dest;
}

void draw_at(int* fb, int x, int y, int color) {
    if (x < 0) return;
    if (y < 0) return;
    if (x >= width) return;
    if (y >= height) return;
    fb[x + y * width] = color | 0xFF000000;
}

void dot_at(int* fb, int x, int y, int color) {
    int size = 10;
    for (int yo = 0; yo < size; yo++) {
        for (int xo = 0; xo < size; xo++) {
            draw_at(fb, x + xo - size / 2, y + yo - size / 2, color);
        }
    }
}

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "Test Drawing Program");

    int* fb = os_map_window_framebuffer(window);

    OSEvent event;

    int i = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fb[i++] = 0xFF000000;
        }
    }

    int colors[] = {
        0xFF0000,
        0x00FF00,
        0x0000FF,
        0xFFFF00,
        0x00FFFF,
        0xFF00FF,
        0xFFFFFF,
    };

    int color_i = 0;

    while (1) {
        os_swap_window_buffers(window);

        os_wait_for_events(); // limit cpu
        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_MOUSE_MOVE) {
                OSMouseEvent* mouse = (OSMouseEvent*) &event;
                int x = mouse->x;
                int y = mouse->y;
                int buttons = mouse->buttons;
                int color = colors[color_i];

                dot_at(fb, x, y, color);

                color_i++;
                color_i %= sizeof(colors) / 4;
            }
        }
    }
    return 0;
}
