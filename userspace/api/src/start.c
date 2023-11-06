#include "os.h"
#include "events.h"
#include "malloc.h"

void os_temp_init() {
    init_events();
    malloc_init(0x1000);
}
