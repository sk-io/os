// doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomgeneric.h"
#include "doomkeys.h"
#include "m_argv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <os.h>
#include <stdbool.h>

int window;
uint32_t* fb;
int shown_buffer;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

void DG_Init() {
    window = os_create_window(DOOMGENERIC_RESX, DOOMGENERIC_RESY, OS_DOUBLE_BUFFERED);
    fb = os_map_window_framebuffer(window);
    shown_buffer = 0;
}

static unsigned char ascii_to_doomkey(unsigned int key) {
    switch (key) {
    case 'w': return KEY_UPARROW;
    case 's': return KEY_DOWNARROW;
    case 'a': return KEY_STRAFE_L;
    case 'd': return KEY_STRAFE_R;

    case 'k': return KEY_LEFTARROW;
    case 'l': return KEY_RIGHTARROW;

    case ' ': return KEY_USE;

    case 0x1b:
        return KEY_ESCAPE;
    case '\n':
        return KEY_ENTER;
    }
    return -1;
}

static void add_key_to_queue(int pressed, unsigned char key) {
    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_DrawFrame() {
    // for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
    //     fb[i] = 0xFF00FF00;
    uint32_t* draw_fb = fb + (shown_buffer == 0 ? (DOOMGENERIC_RESX * DOOMGENERIC_RESY) : 0);

    memcpy(draw_fb, DG_ScreenBuffer, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
        draw_fb[i] |= 0xFF000000;

    shown_buffer = os_swap_window_buffers(window);

    OSEvent event;
    while (os_poll_event(&event)) {
        if (event.type == OS_EVENT_KEYBOARD) {
            OSKeyboardEvent *key_event = (OSKeyboardEvent *) &event;
            int pressed = key_event->state == 1;

            unsigned char key = ascii_to_doomkey(key_event->ascii);
            if (key_event->scancode == 29) {
                key = KEY_FIRE;
                pressed = 1;
            }
            if (key_event->scancode == 157) {
                key = KEY_FIRE;
                pressed = 0;
            }
            
            add_key_to_queue(pressed, key);
        }
    }
}

void DG_SleepMs(uint32_t ms) {
    // todo
}

uint32_t DG_GetTicksMs() {
    return os_get_system_time();
}

int DG_GetKey(int *pressed, unsigned char *doomKey) {
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex) {
        // key queue is empty
        return 0;
    }

    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
}

void DG_SetWindowTitle(const char *title) {
    os_set_window_title(window, title);
}

int main(int argc, char **argv) {
    int num_doom_args = 3;
    char *doom_args[] = {
        "doom.exe",
        "-iwad",
        "DOOM1.WAD",
    };

    doomgeneric_Create(num_doom_args, doom_args);

    for (int i = 0;; i++) {
        doomgeneric_Tick();
    }

    return 0;
}
