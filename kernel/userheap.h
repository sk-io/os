#pragma once

// manage process heap

#include "tasks.h"

void init_user_heap(Task* task);
void set_user_heap_end(Task* task, u32 new_heap_end);
