#include "exceptions.h"

#include "interrupts.h"
#include "util.h"
#include "log.h"
#include "memory.h"
#include "tasks.h"

static void handle_div_by_zero(TrapFrame* frame);
static void handle_page_fault(TrapFrame* frame);
static void handle_general_protection_fault(TrapFrame* frame);

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
    crash_and_burn();
}

void handle_page_fault(TrapFrame* frame) {
    u32 cr2;
    asm volatile(
        "mov %%cr2, %0"
        : "=r"(cr2)
    );

    u32* pd = mem_get_current_page_directory();
    kernel_log("page fault! vaddr=%x eip=%x pdir=%x error=%u", cr2, frame->eip, pd, frame->error);
    crash_and_burn();
}

static void handle_div_by_zero(TrapFrame* frame) {
    kernel_log("div by zero! error=%u eip=%x", frame->error, frame->eip);
    crash_and_burn();
}
