#include "mouse.h"

#include "util.h"
#include "interrupts.h"
#include "log.h"
#include "gui.h"

s32 mouse_x_acc;
s32 mouse_y_acc;
bool mouse_left_button;

#define PACKET_BYTES 3
static u8 buffer[PACKET_BYTES];
static u8 buffer_index;

static void wait_read();
static void wait_write();
static void write_mouse(u8 data);
static void handle_mouse_interrupt(TrapFrame* frame);

void init_mouse() {
    mouse_x_acc = 0;
    mouse_y_acc = 0;

    memset(buffer, 0, PACKET_BYTES);
    buffer_index = 0;
    
    register_isr(IRQ_OFFSET + 12, handle_mouse_interrupt);

    wait_write();
    outb(0x64, 0x20);

    u8 status = inb(0x60);
    status |= 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    outb(0x64, 0xa8);

    write_mouse(0xf4);
}

static void wait_write() {
    s32 try_count = 1000;
    while (((inb(0x64) & 2) != 0) && --try_count > 0);
}

static void write_mouse(u8 data) {
    wait_write();
    outb(0x64, 0xD4);

    wait_write();
    outb(0x60, data);
}

static void handle_mouse_interrupt(TrapFrame* frame) {
    s32 try_count = 1000;
    while (((inb(0x64) & 0x20) == 0) && --try_count > 0);

    u8 data = inb(0x60);

    buffer[buffer_index++] = data;

    if (buffer_index >= PACKET_BYTES) {
        buffer_index = 0;

        mouse_x_acc += (s8) buffer[2];
        mouse_y_acc -= (s8) buffer[0];
        mouse_left_button = buffer[1] & 1;
        if (mouse_left_button)
            gui_left_click = true;
        // kernel_log("mouse x=%d, y=%d", mouse_x_acc, mouse_y_acc);
    }
}
