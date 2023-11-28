#pragma once

#include "printf.h"
#include "types.h"

void init_logging(bool serial);
void kernel_log(const char* format, ...);
void kernel_log_char(char c); // temp

