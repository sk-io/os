#pragma once

#include "types.h"
#include "fat32.h"

typedef struct {
    u32 addr;
    u32 size;
    FAT32_Volume volume;
} Ramdisk;

extern Ramdisk ramdisk;

void init_disks(u32 ramdisk_location, u32 ramdisk_size);
void disk_read_sector(u8* out_buffer, u32 sector);
