#include "disk.h"

#include "util.h"
#include "memory.h"
#include "ata.h"

Ramdisk ramdisk;
FAT32_Volume primary_volume;
bool using_ramdisk = false;

void init_disks(struct multiboot_info* info) {
    memset(&ramdisk, 0, sizeof(Ramdisk));

    u32 mod_count = info->mods_addr;
    if (mod_count > 0) {
        using_ramdisk = true;
        kernel_log("Found a multiboot module, assuming it's a ramdisk :)");

        u32 phys_start = *(u32*) (info->mods_addr);
        u32 phys_end = *(u32*) (info->mods_addr + 4);

        kernel_log("Ramdisk physical addr: %x to %x", phys_start, phys_end);
        kernel_log("Ramdisk size: %u bytes", phys_end - phys_start);
        
        ramdisk.phys_addr = phys_start;
        assert((ramdisk.phys_addr & 0xFFF) == 0);

        ramdisk.size = phys_end - phys_start;
        assert(ramdisk.size < 0x10000000);
    } else {
        using_ramdisk = false;
        kernel_log("No multiboot modules provided. Using ATA drive instead");
        init_ata();
    }
}

void disk_read_sector(u8* out_buffer, u32 sector) {
    if (using_ramdisk) {
        assert((sector + 1) * SECTOR_SIZE < ramdisk.size);
        memcpy(out_buffer, KERNEL_RAMDISK + sector * SECTOR_SIZE, SECTOR_SIZE);
        return;
    }

    ata_read_sector(sector, out_buffer);
}

// requires memory to be initialized
void map_ramdisk() {
    if (!using_ramdisk)
        return;
    
    u32 needed_page_count = CEIL_DIV(ramdisk.size, 0x1000);
	kernel_log("Mapping ramdisk: %u", needed_page_count);
    for (u32 i = 0; i < needed_page_count; i++) {
        u32 offset = i * 0x1000;
        mem_map_page((char*) (KERNEL_RAMDISK + offset), ((u32) ramdisk.phys_addr) + offset, 0);
    }
}
