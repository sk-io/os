#include <os.h>
#include <types.h>
#include <sgfx.h>

#define width 300
#define height 240

#define board_w 12
#define board_h 5

#define ABS(N) (((N)<0)?(-(N)):(N))

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

int wait;

void reset();
void move_ball(Ball* ball, int dx, int dy);
bool ball_intersects(Ball* ball, int x, int y, int w, int h);
void frame();

void reset() {
    for (int i = 0; i < board_w * board_h; i++)
        board[i] = 1;

    ball.x = width / 2;
    ball.y = height / 2;
    ball.dx = ball.dy = 4;
    ball.size = 10;

    // paddle.x = width / 2;
    paddle.y = height - 25;
    paddle.w = 40;
    paddle.h = 10;
    
    wait = 50;
}

void move_ball(Ball* ball, int dx, int dy) {
    ball->x += dx;
    ball->y += dy;

    if (ball->x < 0) {
        ball->x = 0;
        ball->dx *= -1;
    }
    if (ball->x > width - ball->size) {
        ball->x = width - ball->size;
        ball->dx *= -1;
    }
    if (ball->y < 0) {
        ball->y = 0;
        ball->dy *= -1;
    }

    if (ball->y > height) {
        reset();
    }

    if (ball_intersects(ball, paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.w, paddle.h)) {
        if (dx) {
            ball->x -= dx;
            ball->dx *= -1;
        } else {
            ball->y -= dy;
            ball->dy *= -1;

            int delta_x = ABS(ball->x + ball->size / 2 - paddle.x);
            int speed = (delta_x > paddle.w / 3) ? 7 : 4;
            ball->dx = ball->dx > 0 ? speed : -speed;
        }
    }

    for (int y = 0; y < board_h; y++) {
        for (int x = 0; x < board_w; x++) {
            if (board[x + y * board_w] == 0)
                continue;
            
            if (ball_intersects(ball, x * cell_w, y * cell_h, cell_w - 1, cell_h - 1)) {
                board[x + y * board_w] = 0;
                if (dx) {
                    ball->x -= dx;
                    ball->dx *= -1;
                } else {
                    ball->y -= dy;
                    ball->dy *= -1;
                }
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
    if (wait) {
        wait--;
    } else {
        move_ball(&ball, 0, ball.dy);
        move_ball(&ball, ball.dx, 0);
    }

    sgfx_fill(&ctx, 0xff222034);

    for (int y = 0; y < board_h; y++) {
        for (int x = 0; x < board_w; x++) {
            if (board[x + y * board_w])
                sgfx_fill_rect(&ctx, x * cell_w, y * cell_h, cell_w - 1, cell_h - 1, 0xFF00FF00);
        }
    }

    sgfx_fill_rect(&ctx, ball.x, ball.y, ball.size, ball.size, 0xFFFFFFFF);

    sgfx_fill_rect(&ctx, paddle.x - paddle.w / 2, paddle.y - paddle.h / 2, paddle.w, paddle.h, 0xFFFFFFFF);
}

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, OS_DOUBLE_BUFFERED);
    os_set_window_title(window, "Breakout!");

    int* fb = os_map_window_framebuffer(window);

    int shown_buffer = 0;
    OSEvent event;

    sgfx_init(&ctx, fb, width, height);

    paddle.x = width / 2;
    reset();

    while (1) {
        ctx.framebuffer = fb + (shown_buffer == 0 ? (width * height) : 0);
        frame();

        shown_buffer = os_swap_window_buffers(window);

        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_MOUSE_MOVE) {
                OSMouseEvent* mouse = (OSMouseEvent*) &event;
                paddle.x = mouse->x;
            }
        }
    }
    return 0;
}
