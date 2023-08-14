#pragma once

#include "tasks.h"

// manage process heap

void init_user_heap(Task* task);
void set_user_heap_end(Task* task, u32 new_heap_end);
