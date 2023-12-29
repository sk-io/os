#pragma once

#include "types.h"
#include "fat32.h"
#include "multiboot.h"

typedef struct {
    u32 size;
    u32 phys_addr;
} Ramdisk;

extern Ramdisk ramdisk;
extern FAT32_Volume primary_volume;
extern bool using_ramdisk;

void init_disks(struct multiboot_info* info);
void disk_read_sector(u8* out_buffer, u32 sector);
void map_ramdisk();
