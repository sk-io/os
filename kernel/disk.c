#include "disk.h"

#include "util.h"
#include "memory.h"

// TODO: allow for multiple storage devices other than just the ramdisk

Ramdisk ramdisk;

void init_disks(u32 ramdisk_location, u32 ramdisk_size) {
    assert(ramdisk_size < 0x10000000);
    assert((ramdisk_location & 0xFFF) == 0);

    memset(&ramdisk, 0, sizeof(Ramdisk));
    ramdisk.addr = ramdisk_location;
    ramdisk.size = ramdisk_size;
    
    u32 needed_page_count = CEIL_DIV(ramdisk_size, 0x1000);

    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_RAMDISK + offset), ((u32) ramdisk_location) + offset, 0);
    }

    fat32_init_volume(&ramdisk.volume);
}

void disk_read_sector(u8* out_buffer, u32 sector) {
    assert((sector + 1) * SECTOR_SIZE < ramdisk.size);
    memcpy(out_buffer, ramdisk.addr + sector * SECTOR_SIZE, SECTOR_SIZE);
}
