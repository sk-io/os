#pragma once

#include "types.h"

bool init_ata();
void ata_read_sector(u64 lba, u8* out_buffer);
