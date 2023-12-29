#include "ata.h"

#include "util.h"
#include "interrupts.h"
#include "log.h"

#define PRIMARY_DATA    0x1F0
#define PRIMARY_ERROR   0x1F1
#define PRIMARY_SECTORS 0x1F2
#define PRIMARY_LBA_LO  0x1F3
#define PRIMARY_LBA_MI  0x1F4
#define PRIMARY_LBA_HI  0x1F5
#define PRIMARY_DRIVE   0x1F6
#define PRIMARY_STATUS  0x1F7 // also used for sending commands

#define STATUS_BSY  0b10000000
#define STATUS_DRDY 0b01000000
#define STATUS_DF   0b00100000
#define STATUS_DSC  0b00010000
#define STATUS_DRQ  0b00001000
#define STATUS_CORR 0b00000100
#define STATUS_IDX  0b00000010
#define STATUS_ERR  0b00000001

static void wait_for_bsy() {
    while (inb(PRIMARY_STATUS) & STATUS_BSY);
}

static void wait_for_drdy() {
    while (!(inb(PRIMARY_STATUS) & STATUS_DRDY));
}

static void wait_for_drq() {
    while (!(inb(PRIMARY_STATUS) & STATUS_DRQ));
}

void init_ata() {
    VERIFY_INTERRUPTS_DISABLED;

    kernel_log("attempting to init ata drive");
    // kernel_log("status=%x", inb(PRIMARY_STATUS));

    // select drive 0
    outb(PRIMARY_DRIVE, 0xA0);

    // delay
    for (int i = 0; i < 15; i++)
        inb(PRIMARY_STATUS);
    
    outb(PRIMARY_SECTORS, 0);
    outb(PRIMARY_LBA_LO, 0);
    outb(PRIMARY_LBA_MI, 0);
    outb(PRIMARY_LBA_HI, 0);
    outb(PRIMARY_STATUS, 0xEC); // identify drive

    u8 status = inb(PRIMARY_STATUS);
    if (status == 0) {
        kernel_log("Could not identify a valid ATA drive!");
        assert(0);
    }

    wait_for_bsy();
    wait_for_drq();

    kernel_log("status=%02x", inb(PRIMARY_STATUS));

    u16 data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = inw(PRIMARY_DATA);
    }

    bool supports_lba48_mode = (data[83] >> 10) & 1;
    assert(supports_lba48_mode);
}

void ata_read_sector(u64 lba, u8* out_buffer) {
    // kernel_log("[ATA] reading sector %u", lba);

    u16 sectors = 1;
    outb(PRIMARY_DRIVE, 0x40);
    outb(PRIMARY_SECTORS, sectors >> 8);
    outb(PRIMARY_LBA_LO, lba >> (3 * 8));
    outb(PRIMARY_LBA_MI, lba >> (4 * 8));
    outb(PRIMARY_LBA_HI, lba >> (5 * 8));
    outb(PRIMARY_SECTORS, sectors & 0xFF);
    outb(PRIMARY_LBA_LO, lba >> (0 * 8));
    outb(PRIMARY_LBA_MI, lba >> (1 * 8));
    outb(PRIMARY_LBA_HI, lba >> (2 * 8));

    outb(PRIMARY_STATUS, 0x24); // READ SECTORS EXT

    wait_for_bsy();
    wait_for_drq();

    for (int i = 0; i < 256; i++) {
        ((u16*)out_buffer)[i] = inw(PRIMARY_DATA);
    }
}
