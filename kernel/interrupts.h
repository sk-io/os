#pragma once

#include "types.h"

#define IRQ_OFFSET 32

#define enable_interrupts() asm volatile("sti")
#define disable_interrupts() asm volatile("cli")
#define FL_IF 0x200 // interrupt enable

typedef struct {
    u16 isr_low;
    u16 kernel_cs;
    u8  reserved;
    u8  attributes;
    u16 isr_high;
} __attribute__((packed)) IDTEntry;

typedef struct {
    u16 limit;
    u32 base;
} __attribute__((packed)) IDTPointer;

typedef struct {
    // pushed by us:
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax; // esp is ignored
    u32 interrupt, error;

    // pushed by the CPU:
    u32 eip, cs, eflags, usermode_esp, usermode_ss;
} TrapFrame;

typedef struct {
    u32 edi;
    u32 esi;
    u32 ebx;
    u32 ebp;
    u32 eip;
} TaskSwitchContext;

typedef void (*ISRFunction)(TrapFrame*);

void setup_interrupts();
void register_isr(u8 vector, ISRFunction func);
void handle_interrupt(TrapFrame* frame);
