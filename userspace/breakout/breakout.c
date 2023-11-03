#include <os.h>
#include <types.h>
#include "graphics.h"

#define width 300
#define height 240

#define board_w 12
#define board_h 5

const int cell_w = width / board_w;
const int cell_h = 50 / board_h;

u8 board[board_w * board_h];

GraphicsContext ctx;

typedef struct {
    int x, y;
    int dx, dy;
    int size;
} Ball;

typedef struct {
    int x, y;
    int w, h;
} Paddle;

Ball ball; // rodata initialization is broken rn
Paddle paddle;

void reset();
void move_ball(Ball* ball);
bool ball_intersects(Ball* ball, int x, int y, int w, int h);
void frame();

void reset() {
    for (int i = 0; i < board_w * board_h; i++)
        board[i] = 1;

    ball.x = width / 2;
    ball.y = height / 2;
    ball.dx = ball.dy = 4;
    ball.size = 10;

    paddle.x = width / 2;
    paddle.y = height - 25;
    paddle.w = 40;
    paddle.h = 10;
}

void move_ball(Ball* ball) {
    // todo: move in individual axis

    ball->x += ball->dx;
    ball->y += ball->dy;

    if (ball->x < 0) {
        ball->x = 0;
        ball->dx = -ball->dx;
    }
    if (ball->x > width - ball->size) {
        ball->x = width - ball->size;
        ball->dx = -ball->dx;
    }
    if (ball->y < 0) {
        ball->y = 0;
        ball->dy = -ball->dy;
    }

    if (ball->y > height - ball->size) {
        ball->y = height - ball->size;
        ball->dy = -ball->dy;
    }

    if (ball->dy > 0 && ball_intersects(ball, paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.w, paddle.h)) {
        ball->dy = -ball->dy;
    }

    for (int y = 0; y < board_h; y++) {
        for (int x = 0; x < board_w; x++) {
            if (ball_intersects(ball, x * cell_w, y * cell_h, cell_w - 1, cell_h - 1)) {
                board[x + y * board_w] = 0;

            }
        }
    }
}

bool ball_intersects(Ball* ball, int x, int y, int w, int h) {
    if (ball->x > x + w) return false;
    if (ball->y > y + h) return false;
    if (ball->x + ball->size <= x) return false;
    if (ball->y + ball->size <= y) return false;
    return true;
}

void frame() {
    move_ball(&ball);

    graphics_fill(&ctx, 0xff222034);

    for (int y = 0; y < board_h; y++) {
        for (int x = 0; x < board_w; x++) {
            if (board[x + y * board_w])
                graphics_fill_rect(&ctx, x * cell_w, y * cell_h, cell_w - 1, cell_h - 1, 0xFF00FF00);
        }
    }

    graphics_fill_rect(&ctx, ball.x, ball.y, ball.size, ball.size, 0xFFFFFFFF);

    graphics_fill_rect(&ctx, paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.w, paddle.h, 0xFFFFFFFF);
}

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, OS_DOUBLE_BUFFERED);
    os_set_window_title(window, "Breakout!");

    int* fb = os_map_window_framebuffer(window);

    int shown_buffer = 0;
    OSEvent event;

    init_graphics(&ctx, fb, width, height);

    reset();

    while (1) {
        ctx.framebuffer = fb + (shown_buffer == 0 ? (width * height) : 0);
        frame();

        shown_buffer = os_swap_window_buffers(window);

        while (os_poll_event(&event)) {
            if (event.type == EVENT_MOUSE_MOVE) {
                OSMouseEvent* mouse = (OSMouseEvent*) &event;
                paddle.x = mouse->x;
            }
        }
    }
    return 0;
}
