#include "pit.h"

#include "interrupts.h"
#include "util.h"
#include "tasks.h"

u64 system_ticks = 0;

static bool timer_enabled = false;
void handle_timer_int(TrapFrame* frame);

void init_timer(u32 frequency) {
    u32 divisor = 1193180 / frequency;

    outb(0x43, 0x36);

    u8 l = (u8)(divisor & 0xFF);
    u8 h = (u8)(divisor >> 8 & 0xFF);

    outb(0x40, l);
    outb(0x40, h);

    register_isr(32, handle_timer_int);
}

void set_timer_enabled(bool enabled) {
    timer_enabled = enabled;
}

void handle_timer_int(TrapFrame* frame) {
    UNUSED_VAR(frame);
    system_ticks++;

    if (timer_enabled) {
        task_schedule();
    }
}