#pragma once

#include "types.h"
#include "tasks.h"

// userspace events

enum {
    EVENT_NONE,
    EVENT_KEYBOARD,
    EVENT_MOUSE_CLICK,
    EVENT_MOUSE_MOVE,
    EVENT_TIMER,
};

typedef struct {
    uint32_t type;
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
} __attribute__((__packed__)) Event;

// FIXME: for now, keep in sync with os.h

#define OS_SHIFT_HELD (1 << 0)
#define OS_CTRL_HELD  (1 << 1)
#define OS_ALT_HELD   (1 << 2)

typedef struct {
    uint32_t type;
    uint32_t scancode;
    uint16_t flags;
    uint8_t ascii;
    uint8_t state;
} __attribute__((__packed__)) OSKeyboardEvent;

typedef struct {
    uint32_t type;
    uint32_t x;
    uint32_t y;
    uint32_t buttons;
} __attribute__((__packed__)) OSMouseEvent;

typedef struct {
    uint32_t type;
    uint32_t timer_id;
    uint32_t time_of_fire;
    uint32_t data2;
} __attribute__((__packed__)) OSTimerEvent;

#define EVENT_BUFFER_SIZE 256

typedef struct {
    u32 num_events;
    Event buffer[EVENT_BUFFER_SIZE];
} EventBuffer;

void init_events();
void init_events_for_task(Task* task);
void destroy_events_for_task(Task* task);
void handle_event(const Event* event);
void check_event_timers();
