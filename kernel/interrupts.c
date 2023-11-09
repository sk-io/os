#include "interrupts.h"

#include "gdt.h"
#include "util.h"
#include "log.h"
#include "tasks.h"

IDTEntry idt[256] __attribute__((aligned(0x10)));
IDTPointer idt_pointer;
ISRFunction isr_functions[256];

static void set_idt_entry(u8 vector, void* isr, u8 attributes);
static void remap_pic();
// in interrupt_asm
extern void* isr_redirect_table[];
extern void isr128();

static bool cli_init_state;
static s32 cli_level = 0;

void setup_interrupts() {
    memset((u8*) &idt, 0, sizeof(IDTEntry) * 256);
    memset((u8*) &isr_functions, 0, sizeof(ISRFunction) * 256);

    idt_pointer.limit = sizeof(IDTEntry) * 256 - 1;
    idt_pointer.base = (u32) &idt;

    remap_pic();

    // all entries are interrupt gates (interrupts are disabled on entry)
    for (int i = 0; i < 48; i++) {
        set_idt_entry(i, isr_redirect_table[i], 0x8E);
    }
    set_idt_entry(0x80, isr128, 0xEE); // syscalls have DPL 3

    asm volatile("lidt %0" :: "m"(idt_pointer));
}

void set_idt_entry(u8 vector, void* isr, u8 attributes) {
    idt[vector].isr_low    = (u32) isr & 0xFFFF;
    idt[vector].kernel_cs  = GDT_KERNEL_CODE;
    idt[vector].reserved   = 0;
    idt[vector].attributes = attributes;
    idt[vector].isr_high   = (u32) isr >> 16;
}

void remap_pic() {
    // send commands to PIC to remap IRQs
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void register_isr(u8 vector, ISRFunction func) {
    isr_functions[vector] = func;
}

void handle_interrupt(TrapFrame* frame) {
    if (frame->interrupt >= 32 && frame->interrupt <= 47) {
        // acknowledge IRQ
        if (frame->interrupt >= 40) {
            outb(0xA0, 0x20); // slave PIC
        }
        outb(0x20, 0x20);
    }

    if (frame->interrupt == IRQ_OFFSET + 7) {
        // ignore spurious IRQ 7
        return;
    }

    if (frame->interrupt == IRQ_OFFSET + 8) {
        // ignore CMOS real-time clock
        return;
    }

    ISRFunction isr = isr_functions[frame->interrupt];
    if (isr != NULL) {
        isr(frame);
    } else {
        kernel_log("unhandled interrupt %u!", frame->interrupt);
        crash_and_burn();
    }

    // make sure we don't return to a dead task
    if (current_task->state == TASK_STATE_DEAD) {
        enable_interrupts();
        while (true) {
            halt();
        }
    }
}

void push_cli() {
    u32 eflags = read_eflags();

    disable_interrupts();

    if (cli_level == 0) {
        cli_init_state = (eflags & FL_IF) != 0;
    }
    
    cli_level++;
}

void pop_cli() {
    u32 eflags = read_eflags();
    assert((eflags & FL_IF) == 0);

    assert(cli_level > 0);
    cli_level--;

    if (cli_level == 0 && cli_init_state) {
        enable_interrupts();
    }
}

bool are_interrupts_enabled() {
    return read_eflags() & FL_IF;
}
