#include "ramdisk.h"

#include "fatfs/fatfs_diskio.h"
#include "fatfs/fatfs_ff.h"
#include "interrupts.h"
#include "util.h"
#include "log.h"

static struct {
    u32 location;
    u32 size;
    FATFS fs;
} ramdisk;

void init_ramdisk(u32 location, u32 size) {
    kernel_log("Ram disk located at %x with size %u bytes", location, size);
    ramdisk.location = location;
    ramdisk.size = size;

    FRESULT res;

    res = f_mount(&ramdisk.fs, "", 0);

    if (res != FR_OK) {
        kernel_log("f_mount error: %u\n", (u32) res);
        crash_and_burn();
    }
}

DSTATUS disk_initialize(BYTE pdrv) {
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buffer, DWORD sector, UINT count) {
    VERIFY_INTERRUPTS_DISABLED;
    
    uint32_t offset = sector * RAMDISK_BLOCKSIZE;
    uint32_t size = count * RAMDISK_BLOCKSIZE;
    memcpy(buffer, (u8*) (ramdisk.location + offset), size);

    return RES_OK;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buffer, DWORD sector, UINT count) {
    VERIFY_INTERRUPTS_DISABLED;

    uint32_t offset = sector * RAMDISK_BLOCKSIZE;
    uint32_t size = count * RAMDISK_BLOCKSIZE;
    memcpy((u8*) (ramdisk.location + offset), buffer, size);

    return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    VERIFY_INTERRUPTS_DISABLED;
    DRESULT dr = RES_ERROR;

    switch (cmd)
    {
    case CTRL_SYNC:
        dr = RES_OK;
        break;
    case GET_SECTOR_COUNT:
        *(DWORD*) buff = ramdisk.size / RAMDISK_BLOCKSIZE;
        dr = RES_OK;
        // printkf("disk_ioctl GET_SECTOR_COUNT: %d\n", value);
        break;
    case GET_BLOCK_SIZE:
        *(DWORD*) buff = RAMDISK_BLOCKSIZE;
        dr = RES_OK;
        break;
    }

    return dr;
}
