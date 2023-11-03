#pragma once

#include "types.h"

// PS/2 mouse

typedef struct {
    // accumulators
    s32 x_acc;
    s32 y_acc;
    bool left_button;
} Mouse;

extern Mouse mouse;

void init_mouse();
