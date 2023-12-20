#pragma once

#include "types.h"

#define SECTOR_SIZE 512

void disk_read_sector(u8* out_buffer, u32 sector);
