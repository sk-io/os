#include "os.h"

#include <stdint.h>
#include <stddef.h>

void itoa(char *buf, int32_t base, int32_t d) {
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;

    if (base == 'd' && d < 0) {
        *p++ = '-';
        buf++;
        ud = -d;
    } else if (base == 'x') {
        divisor = 16;
    }

    do {
        int remainder = ud % divisor;

        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
    } while (ud /= divisor);

    *p = 0;

    //Reverse BUF.
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2) {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

int sprintf_va(char *buffer, uint32_t buffer_size, uint8_t replace_newline, const char *format, __builtin_va_list vl) {
    char c;
    char buf[20];
    char *p = NULL;

    uint32_t buffer_index = 0;

    while ((c = *format++) != 0) {
        if (c == '\n') {
            if (replace_newline) {
                buf[0] = '\r';
                buf[1] = '\n';
                buf[2] = '\0';
            } else {
                buf[0] = '\n';
                buf[1] = '\0';
            }
            p = buf;

            while (*p) {
                buffer[buffer_index++] = (*p++);
            }
        } else if (c == '%') {
            c = *format++;
            switch (c) {
            case 'p':
            case 'x':
                buf[0] = '0';
                buf[1] = 'x';
                itoa(buf + 2, c, __builtin_va_arg(vl, int));
                p = buf;
                goto string;
                break;
            case 'd':
            case 'u':
                itoa(buf, c, __builtin_va_arg(vl, int));
                p = buf;
                goto string;
                break;

            case 's':
                p = __builtin_va_arg(vl, char *);
                if (!p)
                    p = "(null)";

            string:
                while (*p) {
                    buffer[buffer_index++] = (*p++);
                }
                break;

            default:
                buffer[buffer_index++] = __builtin_va_arg(vl, int);
                break;
            }
        } else {
            buffer[buffer_index++] = c;
        }

        if (buffer_index >= buffer_size - 1) {
            break;
        }
    }

    buffer[buffer_index] = '\0';
    return buffer_index;
}

void os_printf(const char *format, ...) {
    char buffer[1024];

    __builtin_va_list vl;
    __builtin_va_start(vl, format);

    sprintf_va(buffer, 1024, 0, format, vl);

    __builtin_va_end(vl);

    os_print(buffer);
}