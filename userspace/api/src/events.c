#include "os.h"

#include "syscalls.h"

#define EVENT_BUFFER_SIZE 256

typedef struct {
    uint32_t num_events;
    OSEvent buffer[EVENT_BUFFER_SIZE];
} EventBuffer;

static int32_t last_handled_event;
static EventBuffer* event_buffer;

void init_events() {
    last_handled_event = 0;

    int shmem_id = syscall_get_event_buffer_shmem_id();
    // os_printf("my event buffer smobj id is %d", shmem_id);

    event_buffer = syscall_map_shared_mem(shmem_id);
}

int32_t os_poll_event(OSEvent* event) {
    if (last_handled_event == event_buffer->num_events)
        return 0;
    
    *event = event_buffer->buffer[last_handled_event % EVENT_BUFFER_SIZE];
    last_handled_event++;
    return 1;
}

int32_t os_wait_for_events() {
    syscall_wait_for_events();
    return 1;
}

void os_set_timer_interval(int32_t timer_id, int32_t interval_ms) {
    syscall_set_timer_interval(timer_id, interval_ms);
}
