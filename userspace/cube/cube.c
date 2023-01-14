#include <os.h>
#include <types.h>

#include "graphics.h"
#include "math.h"
#include "math3d.h"

#define width 256
#define height 256

GraphicsContext ctx;
int* fb;

double time = 0;

const double cube_verts[] = {
    0, 0, 0,
    1, 0, 0,
    1, 1, 0,
    0, 1, 0,

    0, 0, 1,
    1, 0, 1,
    1, 1, 1,
    0, 1, 1,
};

void line_between(int coords[16], int a, int b, int color) {
    int x0 = coords[a * 2 + 0];
    int y0 = coords[a * 2 + 1];
    int x1 = coords[b * 2 + 0];
    int y1 = coords[b * 2 + 1];
    
    graphics_line(&ctx, x0, y0, x1, y1, color);
}

void frame() {
    vec3 cube_transformed[8];

    for (int i = 0; i < 8; i++) {
        vec3 v = {
            cube_verts[i * 3 + 0] - 0.5,
            cube_verts[i * 3 + 1] - 0.5,
            cube_verts[i * 3 + 2] - 0.5
        };
        v = rotate_x(v, time);
        v = rotate_y(v, time * 1.234);
        v = scale_vec(v, 80);
        cube_transformed[i] = v;
    }

    graphics_fill(&ctx, 0xFF000099);

    int coords[16];
    for (int i = 0; i < 8; i++) {
        double x = cube_transformed[i].x;
        double y = cube_transformed[i].y;

        x += width / 2.0;
        y += height / 2.0;

        coords[i * 2 + 0] = (int) x;
        coords[i * 2 + 1] = (int) y;
    }
    line_between(coords, 0, 1, 0xFFFF00FF);
    line_between(coords, 1, 2, 0xFFFF00FF);
    line_between(coords, 2, 3, 0xFFFF00FF);
    line_between(coords, 3, 0, 0xFFFF00FF);

    line_between(coords, 0 + 4, 1 + 4, 0xFFFF00FF);
    line_between(coords, 1 + 4, 2 + 4, 0xFFFF00FF);
    line_between(coords, 2 + 4, 3 + 4, 0xFFFF00FF);
    line_between(coords, 3 + 4, 0 + 4, 0xFFFF00FF);

    line_between(coords, 0, 4, 0xFFFF00FF);
    line_between(coords, 1, 5, 0xFFFF00FF);
    line_between(coords, 2, 6, 0xFFFF00FF);
    line_between(coords, 3, 7, 0xFFFF00FF);

    time += 0.001;
}

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, OS_DOUBLE_BUFFERED);
    os_set_window_title(window, "3D Cube");

    fb = os_map_window_framebuffer(window);

    int shown_buffer = 0;
    OSEvent event;
    init_graphics(&ctx, fb, width, height);

    while (1) {
        ctx.framebuffer = fb + (shown_buffer == 0 ? (width * height) : 0);
        frame();

        shown_buffer = os_swap_window_buffers(window);

        while (os_poll_event(&event)) {
            if (event.type == EVENT_MOUSE_MOVE) {
                OSMouseEvent* mouse = (OSMouseEvent*) &event;
            }
        }
    }
    return 0;
}
