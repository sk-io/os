#pragma once

#include "syscall_list.h"
#include "types.h"

void init_syscalls();
void register_syscall(u32 vector, void* func);
