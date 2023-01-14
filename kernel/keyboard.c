#include "keyboard.h"

#include "console.h"
#include "interrupts.h"
#include "log.h"
#include "util.h"
#include "tasks.h"
#include "events.h"
#include "kernel.h"

static void handle_key_event(TrapFrame* frame);
static u8 translate_to_ascii(u8 scancode, bool shift, bool alt);

static bool holding_shift = false;

void init_keyboard() {
    register_isr(IRQ_OFFSET + 1, handle_key_event);
}

void handle_key_event(TrapFrame* frame) {
    UNUSED_VAR(frame);

    u8 scancode = 0;
    do {
        scancode = inb(0x64);
    } while ((scancode & 0x01) == 0);

    scancode = inb(0x60);

    if ((scancode & 0b01111111) == 42) {
        holding_shift = (scancode >> 7) == 0;
        return;
    }

    char key = translate_to_ascii(scancode, holding_shift, false);

    Event event;
    event.type = EVENT_KEYBOARD;
    event.data0 = scancode;
    event.data1 = key;
    event.data2 = scancode >> 7 == 0;
    handle_event(&event);
    // kernel_log("key scancode: %u", scancode);
}

static char scancode_map[128] = {
    0,   27,   '1', '2',  '3',  '4',  '5',  '6', '7',
    '8', '9',  '0', '-',  '=',  '\b', '\t', /* <-- Tab */
    'q', 'w',  'e', 'r',  't',  'y',  'u',  'i', 'o',
    'p', '[',  ']', '\n', 0, /* <-- control key */
    'a', 's',  'd', 'f',  'g',  'h',  'j',  'k', 'l',
    ';', '\'', '`', 0,    '\\', 'z',  'x',  'c', 'v',
    'b', 'n',  'm', ',',  '.',  '/',  0,    '*', 0, /* Alt */
    ' ',                                            /* Space bar */
    0,                                              /* Caps lock */
    0,                                              /* 59 - F1 key ... > */
    0,   0,    0,   0,    0,    0,    0,    0,   0, /* < ... F10 */
    0,                                              /* 69 - Num lock*/
    0,                                              /* Scroll Lock */
    0,                                              /* Home key */
    0,                                              /* Up Arrow */
    0,                                              /* Page Up */
    '-', 0,                                         /* Left Arrow */
    0,   0,                                         /* Right Arrow */
    '+', 0,                                         /* 79 - End key*/
    0,                                              /* Down Arrow */
    0,                                              /* Page Down */
    0,                                              /* Insert Key */
    0,                                              /* Delete Key */
    0,   0,    0,   0,                              /* F11 Key */
    0,                                              /* F12 Key */
    0, /* All other keys are undefined */
};

unsigned char scancode_map_shift[128] = {
    0,    27,  '!',  '\"', '#',  0 /* shift+4 */,
    '%',  '&', '/',  '(',        /* 9 */
    ')',  '=', '?',  '`',  '\b', /* Backspace */
    '\t',                        /* Tab */

    'Q',  'W', 'E',  'R', /* 19 */
    'T',  'Y', 'U',  'I',  'O',  'P',
    'A',  'A', '\n', /* Enter key */
    0,               /* 29   - Control */
    'A',  'S', 'D',  'F',  'G',  'H',
    'J',  'K', 'L',  'O', /* 39 */
    '\'', '>', 0,         /* Left shift */
    '*',  'Z', 'X',  'C',  'V',  'B',
    'N',                      /* 49 */
    'M',  ';', ':',  '_',  0, /* Right shift */

    '*',  0, /* Alt */
    ' ',     /* Space bar */
    0,       /* Caps lock */
    0,       /* 59 - F1 key ... > */
    0,    0,   0,    0,    0,    0,
    0,    0,   0,       /* < ... F10 */
    0,                  /* 69 - Num lock*/
    0,                  /* Scroll Lock */
    0,                  /* Home key */
    0,                  /* Up Arrow */
    0,                  /* Page Up */
    '-',  0,            /* Left Arrow */
    0,    0,            /* Right Arrow */
    '+',  0,            /* 79 - End key*/
    0,                  /* Down Arrow */
    0,                  /* Page Down */
    0,                  /* Insert Key */
    0,                  /* Delete Key */
    0,    0,   '>',  0, /* F11 Key */
    0,                  /* F12 Key */
    0,                  /* All other keys are undefined */
};

unsigned char scancode_map_alt[128] = {
    0,    27,  0 /*alt+1*/, '@', 0,    '$', 0,   0,   '{',  '[', /* 9 */
    ']',  '}', '\\',        '=', '\b',                           /* Backspace */
    '\t',                                                        /* Tab */
    'q',  'w', 'e',         'r',                                 /* 19 */
    't',  'y', 'u',         'i', 'o',  'p', '[', ']', '\n',      /* Enter key */
    0, /* 29   - Control */
    'a',  's', 'd',         'f', 'g',  'h', 'j', 'k', 'l',  ';', /* 39 */
    '\'', '`', 0,                                        /* Left shift */
    '\\', 'z', 'x',         'c', 'v',  'b', 'n',         /* 49 */
    'm',  ',', '.',         '/', 0,                      /* Right shift */
    '*',  0,                                             /* Alt */
    ' ',                                                 /* Space bar */
    0,                                                   /* Caps lock */
    0,                                                   /* 59 - F1 key ... > */
    0,    0,   0,           0,   0,    0,   0,   0,   0, /* < ... F10 */
    0,                                                   /* 69 - Num lock*/
    0,                                                   /* Scroll Lock */
    0,                                                   /* Home key */
    0,                                                   /* Up Arrow */
    0,                                                   /* Page Up */
    '-',  0,                                             /* Left Arrow */
    0,    0,                                             /* Right Arrow */
    '+',  0,                                             /* 79 - End key*/
    0,                                                   /* Down Arrow */
    0,                                                   /* Page Down */
    0,                                                   /* Insert Key */
    0,                                                   /* Delete Key */
    0,    0,   '|',         0,                           /* F11 Key */
    0,                                                   /* F12 Key */
    0, /* All other keys are undefined */
};

u8 translate_to_ascii(u8 scancode, bool shift, bool alt) {
    if (shift && alt)
        return 0;
    
    scancode &= 127;

    if (shift)
        return scancode_map_shift[scancode];

    if (alt)
        return scancode_map_alt[scancode];
    
    return scancode_map[scancode];
}
