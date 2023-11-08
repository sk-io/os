#include "exceptions.h"

#include "interrupts.h"
#include "util.h"
#include "log.h"
#include "memory.h"
#include "tasks.h"

static void handle_div_by_zero(TrapFrame* frame);
static void handle_page_fault(TrapFrame* frame);
static void handle_general_protection_fault(TrapFrame* frame);
static void crash(TrapFrame* frame);
static void print_stack_trace(int max_depth, u32 ebp);

void init_exceptions() {
    register_isr(0, handle_div_by_zero);
    register_isr(13, handle_general_protection_fault);
    register_isr(14, handle_page_fault);
}

void handle_general_protection_fault(TrapFrame* frame) {
    u32* pd = mem_get_current_page_directory();
    kernel_log("general protection fault! error=%u eip=%x", frame->error, frame->eip);
    kernel_log("cs=%u ds=%u ss=%u eflags=%u", frame->cs, frame->ds, frame->usermode_ss, frame->eflags);
    kernel_log("pdir=%x", pd);
    kernel_log("current task id=%u", current_task->id);
    
    crash(frame);
}

void handle_page_fault(TrapFrame* frame) {
    u32 cr2;
    asm volatile(
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    u32* pd = mem_get_current_page_directory();
    kernel_log("page fault! vaddr=%x eip=%x pdir=%x error=%u", cr2, frame->eip, pd, frame->error);

    crash(frame);
}

static void handle_div_by_zero(TrapFrame* frame) {
    kernel_log("div by zero! error=%u eip=%x", frame->error, frame->eip);

    crash(frame);
}

static void crash(TrapFrame* frame) {
    print_stack_trace(8, frame->ebp);

    if (frame->eip >= KERNEL_START) {
        crash_and_burn();
        return;
    }

    kill_task(current_task->id);
    task_schedule();
}

typedef struct _StackFrame {
    struct _StackFrame* ebp;
    u32 eip;
} StackFrame;

void print_stack_trace(int max_depth, u32 ebp) {
    kernel_log("stack trace:");
    
    StackFrame *frame = (StackFrame*) ebp;
    for (u32 i = 0; frame && i < max_depth; i++) {
        kernel_log("  0x%x", frame->eip);
        frame = frame->ebp;
    }
}
