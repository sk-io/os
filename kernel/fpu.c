#include "fpu.h"

#include "types.h"
#include "log.h"

// from https://github.com/stevej/osdev/blob/master/kernel/devices/fpu.c

void set_fpu_cw(const u16 cw) {
	asm volatile("fldcw %0" :: "m"(cw));
}

void enable_fpu() {
	kernel_log("Enabling floating-point arithmetic unit");
	size_t cr4;
	asm volatile ("mov %%cr4, %0" : "=r"(cr4));
	cr4 |= 0x200;
	asm volatile ("mov %0, %%cr4" :: "r"(cr4));
	set_fpu_cw(0x37F);
}
