#include "os.h"
#include "events.h"

extern int main(int argc, char* argv[]);

void _start() {
    init_events();
    main(0, 0);
    os_exit();
}
