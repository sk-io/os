#include "util.h"

#include "interrupts.h"

#include "log.h"

#include "gui.h"
#include "graphics.h"

void* memset(void* vdest, u8 val, u32 len) {
    u8* dest = (u8*) vdest;
    u8* temp = (u8*) dest;
    for (; len != 0; len--) *temp++ = val;
    return dest;
}

void* memcpy(void* dest, const void* src, u32 len) {
    const u8* sp = (const u8*) src;
    u8* dp = (u8*) dest;
    for (; len != 0; len--) *dp++ = *sp++;
    return dest;
}

u32 strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

u32 strncmp(const char* s1, const char* s2, u32 n) {
    register unsigned char u1, u2;

    while (n-- > 0) {
        u1 = (unsigned char)*s1++;
        u2 = (unsigned char)*s2++;
        if (u1 != u2) return u1 - u2;
        if (u1 == '\0') return 0;
    }
    return 0;
}

char* strncpy(char* dst, const char* src, u32 n) {
    if (n != 0) {
        char* d = dst;
        const char* s = src;

        do {
            if ((*d++ = *s++) == 0) {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0) *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dst);
}

void crash_and_burn() {
    graphics_fill(0x0000FF);
    draw_debug_console(0xFFFFFF);
    graphics_copy_backbuffer();

    disable_interrupts();
    while (1) {
        asm volatile("hlt");
    }
}

void kernel_assert(int condition, const char* file, int line, const char *error_msg) {
    if (!condition) {
        kernel_log("%s:%d assert failed: %s", file, line, error_msg);
        // hack to print stack trace
        int g = 0;
        g /= g;
        crash_and_burn();
    }
}
