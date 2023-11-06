#include "os.h"

#include "../../../kernel/syscall_list.h"
#include "shmem.h"

#define EVENT_BUFFER_SIZE 256

typedef struct {
    unsigned int num_events;
    OSEvent buffer[EVENT_BUFFER_SIZE];
} EventBuffer;

static int last_handled_event;
static EventBuffer* event_buffer;

void init_events() {
    last_handled_event = 0;

    int shmem_id;
    asm volatile(
        "int $0x80"
        : "=a"(shmem_id) : "a"(SYSCALL_GET_EVENT_BUFFER_SHMEM_ID)
    );

    // os_printf("my event buffer smobj id is %d", shmem_id);

    event_buffer = map_shared_mem(shmem_id);
}

int os_poll_event(OSEvent* event) {
    if (last_handled_event == event_buffer->num_events)
        return 0;
    
    *event = event_buffer->buffer[last_handled_event % EVENT_BUFFER_SIZE];
    last_handled_event++;
    return 1;
}

int os_wait_for_events() {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_WAIT_FOR_EVENT)
    );

    return 1;
}

void os_set_timer_interval(int timer_id, int interval_ms) {
    asm volatile(
        "int $0x80"
        :: "a"(SYSCALL_SET_TIMER_INTERVAL), "b"(timer_id), "c"(interval_ms)
    );
}
