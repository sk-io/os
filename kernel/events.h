#pragma once

#include "types.h"
#include "tasks.h"

enum {
    EVENT_NONE,
    EVENT_KEYBOARD,
    EVENT_MOUSE,
    EVENT_MOUSE_MOVE,
};

typedef struct {
    u32 type;
    u32 data0;
    u32 data1;
    u32 data2;
} Event;

typedef struct {
} EventKeyboard;

#define EVENT_BUFFER_SIZE 256

typedef struct {
    u32 num_events;
    Event buffer[EVENT_BUFFER_SIZE];
} EventBuffer;

void init_events();
void init_events_for_task(Task* task);
void destroy_events_for_task(Task* task);
void handle_event(const Event* event);
