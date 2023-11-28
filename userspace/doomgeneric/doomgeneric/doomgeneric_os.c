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
int *fb;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

void DG_Init() {
    window = os_create_window(DOOMGENERIC_RESX, DOOMGENERIC_RESY, 0);
    fb = os_map_window_framebuffer(window);
}

static unsigned char ascii_to_doomkey(unsigned int key) {
    switch (key) {
    case 'w': return KEY_UPARROW;
    case 's': return KEY_DOWNARROW;
    case 'a': return KEY_STRAFE_L;
    case 'd': return KEY_STRAFE_R;

    case 'k': return KEY_LEFTARROW;
    case 'l': return KEY_RIGHTARROW;
    case 'u': return KEY_FIRE;

    case 'o':
    case 0x1b:
        return KEY_ESCAPE;
    case 'p':
        return KEY_ENTER;
    }
    return -1;
}

static void add_key_to_queue(int pressed, unsigned int keyCode) {
    unsigned char key = ascii_to_doomkey(keyCode);

    unsigned short keyData = (pressed << 8) | key;

    s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
    s_KeyQueueWriteIndex++;
    s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_DrawFrame() {
    // for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
    //     fb[i] = 0xFF00FF00;
    memcpy(fb, DG_ScreenBuffer, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
        fb[i] |= 0xFF000000;

    os_swap_window_buffers(window);

    OSEvent event;
    while (os_poll_event(&event)) {
        if (event.type == OS_EVENT_KEYBOARD) {
            OSKeyboardEvent *key_event = (OSKeyboardEvent *) &event;
            add_key_to_queue(key_event->state == 1, key_event->ascii);
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
