#pragma once

#include "types.h"
#include "fatfs/fatfs_ff.h"

#define RAMDISK_BLOCKSIZE 512

void init_ramdisk(u32 location, u32 size);
