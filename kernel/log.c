#include "log.h"

#include "serial.h"
#include "console.h"

#include <stdarg.h>

void init_logging(bool serial) {
    if (serial) {
        serial_init();
    }

    init_console();
}

void kernel_log(const char* format, ...) {
    char buffer[256];

    va_list args;
    va_start(args, format);
    int num = vsnprintf(buffer, sizeof(buffer) - 2, format, args);
    va_end(args);

    buffer[num] = '\n';
    buffer[num + 1] = 0;

    console_print(buffer);
    for (int i = 0; i < num; i++) {
        serial_write(buffer[i]);
    }
    serial_write('\n');
}

void _putchar(char character) {
    serial_write(character);
}
