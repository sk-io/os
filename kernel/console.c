#include "console.h"

#include "util.h"
#include "shell.h"
#include "gui.h"
#include "kernel.h"

static Console console;

static void vga_clear();
void vga_write_char(char c);
void vga_scroll_up();
void vga_newline();
void update_cursor_pos();
void clear_prompt();

void init_console() {
    console.width = 80;
    console.height = graphics_enabled ? 50 : 25;
    console.cursor_x = 0;
    console.cursor_y = 0;
    console.buffer = graphics_enabled ? (u8*)gui.fake_console_buffer : (u8*)0xC00B8000;
    console.clear_color = 0x0F;

    clear_prompt();
    vga_clear();
    update_cursor_pos();
}

void console_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_write_char(str[i]);
    }

    update_cursor_pos();
}

void console_key_typed(char c) {
    if (c == '\b') {
        if (console.prompt_pos <= 0)
            return;
        console.prompt_buffer[console.prompt_pos--] = 0;
    } else if (console.prompt_pos == sizeof(console.prompt_buffer) - 1) {
        return;
    } else {
        console.prompt_buffer[console.prompt_pos++] = c;
    }

    vga_write_char(c);
    if (c == '\n') {
        shell_execute(console.prompt_buffer);
        clear_prompt();
        console_print("> ");
    }

    update_cursor_pos();

    if (graphics_enabled)
        gui.needs_update = true;
}

void console_set_prompt_enabled(bool enabled) {
    if (enabled) {
        console_print("> ");
    }
}

void vga_clear() {
    u16 c = console.clear_color << 8;
    for (int i = 0; i < console.width * console.height; i++) {
        ((u16*) console.buffer)[i] = c;
    }
}

void vga_write_char(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }

    if (c == '\b') {
        console.cursor_x--;
        u32 i = console.cursor_x + console.cursor_y * console.width;
        console.buffer[i << 1] = ' ';
        return;
    }

    u32 i = console.cursor_x + console.cursor_y * console.width;
    console.buffer[i << 1] = c;
    console.cursor_x++;
    if (console.cursor_x == console.width) {
        vga_newline();
    }
}

void vga_newline() {
    console.cursor_x = 0;
    console.cursor_y++;

    if (console.cursor_y >= console.height) {
        vga_scroll_up();
    }
}

void vga_scroll_up() {
    console.cursor_y--;

    for (int y = 0; y < console.height - 1; y++) {
        for (int x = 0; x < console.width; x++) {
            u32 current = x + y * console.width;
            u32 next = x + (y + 1) * console.width;

            current <<= 1;
            next <<= 1;

            console.buffer[current] = console.buffer[next];
            console.buffer[current + 1] = console.buffer[next + 1];
        }
    }

    // clear last line
    for (int x = 0; x < console.width; x++) {
        u32 current = x + (console.height - 1) * console.width;

        current <<= 1;

        console.buffer[current] = 0x00;
        console.buffer[current + 1] = console.clear_color;
    }
}

void update_cursor_pos() {
    u16 pos = console.cursor_y * console.width + console.cursor_x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (u8) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (u8) ((pos >> 8) & 0xFF));
}

void vga_set_cursor_enabled(bool enabled) {
    UNUSED_VAR(enabled);
}

void clear_prompt() {
    console.prompt_pos = 0;
    memset(console.prompt_buffer, 0, sizeof(console.prompt_buffer));
}