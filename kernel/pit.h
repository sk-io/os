#pragma once

#include "types.h"

typedef struct {
    u64 ticks;
    u32 freq;
    bool enabled;
} PIT;

extern PIT pit;

void init_timer(u32 frequency);
void set_timer_enabled(bool enabled);
