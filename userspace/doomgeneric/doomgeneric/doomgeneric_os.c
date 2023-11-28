//doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdbool.h>
#include <os.h>

int window;
int* fb;

void DG_Init(){
    window = os_create_window(DOOMGENERIC_RESX, DOOMGENERIC_RESY, 0);
    fb = os_map_window_framebuffer(window);
//   window = SDL_CreateWindow("DOOM",
//                             SDL_WINDOWPOS_UNDEFINED,
//                             SDL_WINDOWPOS_UNDEFINED,
//                             DOOMGENERIC_RESX,
//                             DOOMGENERIC_RESY,
//                             SDL_WINDOW_SHOWN
//                             );

//   // Setup renderer
//   renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);
//   // Clear winow
//   SDL_RenderClear( renderer );
//   // Render the rect to the screen
//   SDL_RenderPresent(renderer);

//   texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, DOOMGENERIC_RESX, DOOMGENERIC_RESY);
}

void DG_DrawFrame()
{
    // fb[

    // for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
    //     fb[i] = 0xFF00FF00;
    memcpy(fb, DG_ScreenBuffer, DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);
    for (int i = 0; i < DOOMGENERIC_RESX * DOOMGENERIC_RESY; i++)
        fb[i] |= 0xFF000000;
    
    os_swap_window_buffers(window);

    OSEvent event;
    while (os_poll_event(&event)) {
    }
}

void DG_SleepMs(uint32_t ms)
{
//   SDL_Delay(ms);
}

uint32_t DG_GetTicksMs()
{
//   return SDL_GetTicks();
    return os_get_system_time();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
//   if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
//     //key queue is empty
//     return 0;
//   }else{
//     unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
//     s_KeyQueueReadIndex++;
//     s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

//     *pressed = keyData >> 8;
//     *doomKey = keyData & 0xFF;

//     return 1;
//   }

  return 0;
}

void DG_SetWindowTitle(const char * title)
{
//   if (window != NULL){
//     SDL_SetWindowTitle(window, title);
//   }
}

int main(int argc, char **argv)
{
    int num_doom_args = 3;
    char* doom_args[] = {
        "doom.exe",
        "-iwad",
        "DOOM1.WAD",
    };

    doomgeneric_Create(num_doom_args, doom_args);

    for (int i = 0; ; i++)
    {
        doomgeneric_Tick();
    }

    return 0;
}