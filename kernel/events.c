#include "events.h"

#include "sharedmem.h"
#include "tasks.h"
#include "syscall.h"
#include "interrupts.h"
#include "util.h"
#include "gui.h"
#include "windows.h"
#include "console.h"
#include "time.h"
#include "log.h"

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
    push_cli();

    // should this be in gui.c?

    if (event->type == EVENT_MOUSE_MOVE) {
        Window* w = get_window(window_under_cursor);
        if (w != NULL) {
            send_event_to_task(w->owner_task_id, event);
        }
    }
    
    if (event->type == EVENT_MOUSE_CLICK || event->type == EVENT_KEYBOARD) {
        Window* w = get_window(focused_window);

        if (w != NULL) {
            send_event_to_task(w->owner_task_id, event);
        } else if (event->type == EVENT_KEYBOARD) {
            if (event->data0 >> 7 == 0 && event->data1 != 0)
                console_key_typed(event->data1);
        }
    }
    // todo: send events to tasks listening in the background

    pop_cli();
}

static void send_event_to_task(s32 task_id, const Event* event) {
    assert(task_id > 1000);
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

typedef struct {
    unsigned int type;
    unsigned int timer_id;
    unsigned int time_of_fire;
    unsigned int data2;
} OSTimerEvent;

void check_event_timers() {
    u64 time = get_system_time_millis();

    for (int i = 0; i < MAX_TASKS; i++) {
        Task* task = &tasks[i];
        if (task->state == TASK_STATE_DEAD)
            continue;
        
        for (int t = 0; t < MAX_TIMERS; t++) {
            Timer* timer = &task->timers[t];

            if (!timer->active)
                continue;
            
            if (time >= timer->next_fire) {
                OSTimerEvent event;
                event.type = EVENT_TIMER;
                event.timer_id = t;
                event.time_of_fire = timer->next_fire;
                event.data2 = 0;
                send_event_to_task(task->id, (const Event*) &event);

                timer->next_fire += timer->interval;
            }
        }
    }
}
