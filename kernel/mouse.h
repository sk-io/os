#pragma once

// ps/2 mouse

#include "types.h"

// accumulators
extern s32 mouse_x_acc;
extern s32 mouse_y_acc;
extern bool mouse_left_button;

void init_mouse();
