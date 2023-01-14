#include "serial.h"

#include "types.h"
#include "util.h"
#include "printf.h"

#define PORT 0x3F8   // COM1

static bool serial_initialized = false;

void serial_init() {
    outb(PORT + 1, 0x00);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    // outb(PORT + 1, 0x01);    // Enable interrupts

    serial_initialized = true;
}

static int port_is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

static void port_write(char a) {
   while (port_is_transmit_empty() == 0);
   outb(PORT,a);
}

void serial_write(char c) {
    if (!serial_initialized)
        return;
    
    port_write(c);
}
