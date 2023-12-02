#include <os.h>

extern int main(int argc, char* argv[]);

// TODO: send pointer to data directly somehow
#define USER_STACK_PAGES 16
#define USER_STACK_BOTTOM    0xB0000000
#define TASK_INIT_DATA       (USER_STACK_BOTTOM - (USER_STACK_PAGES + 1) * 0x1000)

void _start() {
    // parse init data (just arguments for now)
    const int max_args = 32;
    int argc = 0;
    char* argv[max_args];

    const char* str = (const char*) TASK_INIT_DATA;
    const char* str_start = str;
    while (1) {
        if (*str == '\0') {
            if (str - str_start == 0)
                break; // empty string; we're done
            
            argv[argc++] = str_start;
            if (argc == max_args) {
                os_print("too many args!\n");
                return;
            }
            str_start = str + 1;
        }

        str++;
        if (str >= USER_STACK_BOTTOM) {
            os_print("failed to parse argv\n");
            return;
        }
    }

    os_temp_init();
    main(argc, argv);
    os_exit();
}
