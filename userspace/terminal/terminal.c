#include <os.h>
#include <types.h>
#include <sgfx.h>

#define width 360
#define height 240

#define textbuffer_width (width/10)
#define textbuffer_height (height/10)

char textbuffer[textbuffer_width * textbuffer_height];
int cursor_x = 2;
int cursor_y = 0;
GraphicsContext ctx;

int main(int argc, char* argv[]) {
    int window = os_create_window(width, height, 0);
    os_set_window_title(window, "Terminal");
    int* fb = os_map_window_framebuffer(window);

    sgfx_init(&ctx, fb, width, height);

    OSEvent event;

    for (int i = 0; i < textbuffer_width * textbuffer_height; i++)
        textbuffer[i] = 0;
    
    textbuffer[0] = '>';

    while (1) {
        sgfx_fill(&ctx, 0xFF000000);
        int i = 0;
        for (int y = 0; y < textbuffer_height; y++) {
            for (int x = 0; x < textbuffer_width; x++) {
                char c = textbuffer[i++];
                if (c == 0)
                    continue;
                
                sgfx_draw_char(&ctx, c, x * 10, y * 10 + 2, 0xFFFFFFFF);
            }
        }

        // sgfx_draw_string("> ", 3, 5, 0xFFFFFFFF);

        os_swap_window_buffers(window);
        os_wait_for_events();
        
        while (os_poll_event(&event)) {
            if (event.type == OS_EVENT_KEYBOARD) {
                OSKeyboardEvent* e = (OSKeyboardEvent*) &event;

                if (e->state)
                    textbuffer[cursor_x++] = (char) e->ascii;
            }
        }
    }
    return 0;
}
