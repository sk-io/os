#include "gdt.h"

#include "util.h"

static GDTEntry gdt_entries[NUM_GDT_ENTRIES];
static GDTPointer gdt_pointer;
static TSS tss;

extern void flush_gdt(u32 gdt_pointer);
extern void flush_tss();

static void set_gdt_entry(u32 num, u32 base, u32 limit, u8 access, u8 flags) {
    gdt_entries[num].base_low    = (base & 0xFFFF);
    gdt_entries[num].base_mid    = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low   = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    
    gdt_entries[num].granularity |= flags & 0xF0;
    gdt_entries[num].access      = access;
}

void setup_gdt() {
    gdt_pointer.limit = NUM_GDT_ENTRIES * 8 - 1;
    gdt_pointer.base = (u32) &gdt_entries;

    memset(&tss, 0, sizeof(tss));
    tss.ss0 = GDT_KERNEL_DATA;

    set_gdt_entry(0, 0, 0, 0, 0);                // 0x00: null
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xC0); // 0x08: kernel text
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xC0); // 0x10: kernel data
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xC0); // 0x18: User mode code segment
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xC0); // 0x20: User mode data segment
    set_gdt_entry(5, (u32) &tss, sizeof(tss), 0x89, 0x40); // 0x28: tss

    flush_gdt((u32) &gdt_pointer);
    flush_tss();
}

void update_tss_esp0(u32 esp0) {
    tss.esp0 = esp0;
}
