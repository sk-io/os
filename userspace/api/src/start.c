#include "os.h"
#include "events.h"
#include "malloc.h"

// extern int main(int argc, char* argv[]);

// void _start() {
//     init_events();
//     malloc_init(0x1000);
//     main(0, 0);
//     os_exit();
// }

void os_temp_init() {
    init_events();
    malloc_init(0x1000);
}
