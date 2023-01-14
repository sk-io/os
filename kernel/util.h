#pragma once

#include "types.h"

void* memset(void* vdest, u8 val, u32 len);
void* memcpy(void* dest, const void* src, u32 len);
u32 strcmp(const char* s1, const char* s2);
u32 strncmp(const char* s1, const char* s2, u32 n);
char* strncpy(char* dst, const char* src, u32 n);

static inline void outb(u16 port, u8 value) {
    asm volatile("outb %1, %0" :: "dN" (port), "a" (value));
}

static inline u8 inb(u16 port) {
    u8 ret;
    asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
    return ret;
}

#define UNUSED_VAR(x) (void) x
#define halt() asm volatile("hlt")

void crash_and_burn();
void kernel_assert(int condition, const char* file, int line, const char* error_msg);

// todo: pass vargs
#define assert_msg(x, msg) kernel_assert(x, __FILE__, __LINE__, msg)
#define assert(x) kernel_assert(x, __FILE__, __LINE__, "")

#define CEIL_DIV(a, b) (((a) + (b) - 1) / (b))
#define CLAMP(x, a, b) (((x) < (a)) ? (a) : ((x) > (b) ? (b) : (x)))

static inline u32 read_eflags() {
    u32 eflags;
    asm volatile(
        "pushfl \n"
        "popl %0 \n"
        : "=r" (eflags)
    );
    return eflags;
}
