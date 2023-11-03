#include "time.h"

#include "pit.h"

u64 get_system_time_millis() {
    // temporary
    return pit.ticks;
}
