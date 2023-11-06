#include <os.h>

extern int main(int argc, char* argv[]);

void _start() {
    os_temp_init();
    main(0, 0);
    os_exit();
}
