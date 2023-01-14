#include "events.h"

#include "sharedmem.h"
#include "tasks.h"
#include "syscall.h"
#include "interrupts.h"
#include "util.h"
#include "gui.h"
#include "windows.h"
#include "console.h"

static u32 get_event_buffer_shmem_id();
static void task_wait_for_event();
static void send_event_to_task(s32 task_id, const Event* event);

void init_events() {
    register_syscall(SYSCALL_GET_EVENT_BUFFER_SHMEM_ID, get_event_buffer_shmem_id);
    register_syscall(SYSCALL_WAIT_FOR_EVENT, task_wait_for_event);
}

void init_events_for_task(Task* task) {
    task->event_shmem_id = sharedmem_create(sizeof(EventBuffer));

    EventBuffer* buffer = sharedmem_map(task->event_shmem_id, true);
    memset(buffer, 0, sizeof(EventBuffer));
    task->event_buffer = buffer;
}

void destroy_events_for_task(Task* task) {
    assert(task->event_shmem_id >= 0);
    
    sharedmem_unmap(task->event_shmem_id, task->event_buffer);
    sharedmem_destroy(task->event_shmem_id);
    task->event_shmem_id = -1;
}

void handle_event(const Event* event) {
    u32 eflags = read_eflags();
    if (eflags & FL_IF)
        disable_interrupts();

    // should this be in gui.c?
    Window* w = get_window(focused_window);
    if (w != NULL) {
        assert(w->owner_task_id > 1000);
        send_event_to_task(w->owner_task_id, event);
    } else if (event->type == EVENT_KEYBOARD) {
        if (event->data0 >> 7 == 0 && event->data1 != 0)
            console_key_typed(event->data1);
    }

    // todo: send events to tasks listening in the background

    if (eflags & FL_IF)
        enable_interrupts();
}

static void send_event_to_task(s32 task_id, const Event* event) {
    Task* task = get_task(task_id);
    assert(task->state != TASK_STATE_DEAD);
    assert(task->event_buffer);

    EventBuffer* buffer = task->event_buffer;
    buffer->buffer[buffer->num_events % EVENT_BUFFER_SIZE] = *event;
    buffer->num_events++;

    if (task->state == TASK_STATE_WAIT_FOR_EVENT)
        task->state = TASK_STATE_READY;
}

u32 get_event_buffer_shmem_id() {
    return current_task->event_shmem_id;
}

static void task_wait_for_event() {
    current_task->state = TASK_STATE_WAIT_FOR_EVENT;
    
    task_schedule();
    assert(current_task->state != TASK_STATE_WAIT_FOR_EVENT);
}
