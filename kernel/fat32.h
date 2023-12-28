#pragma once

#include "types.h"

#define SECTOR_SIZE 512

#pragma pack(push, 1)

typedef struct {
    u8 bootjmp[3];
    u8 oem_name[8];
    u16 bytes_per_sector;
    u8 sectors_per_cluster;
    u16 reserved_sector_count;
    u8 table_count;
    u16 root_entry_count;
    u16 total_sectors_16;
    u8 media_type;
    u16 table_size_16;
    u16 sectors_per_track;
    u16 head_side_count;
    u32 hidden_sector_count;
    u32 total_sectors_32;

    // extended fat32 stuff
    u32 table_size_32;
    u16 extended_flags;
    u16 fat_version;
    u32 root_cluster;
    u16 fat_info;
    u16 backup_BS_sector;
    u8 reserved_0[12];
    u8 drive_number;
    u8 reserved_1;
    u8 boot_signature;
    u32 volume_id;
    u8 volume_label[11];
    u8 fat_type_label[8];
} FAT32_Header;

typedef struct {
    u8 short_name[8];
    u8 short_ext[3];
    u8 attrib;
    u8 uhhh;
    u8 uhhhhhhh;
    u16 creation_time;
    u16 creation_date;
    u16 access_date;
    u16 cluster_high;
    u16 modified_time;
    u16 modified_date;
    u16 cluster_low;
    u32 file_size; // bytes
} FAT32_Directory_Entry;

typedef struct {
    u8 sequence;
    u16 name0[5];
    u8 attrib;
    u8 type;
    u8 checksum;
    u16 name1[6];
    u16 cluster;
    u16 name2[2];
} FAT32_Directory_Entry_LFN;

// static_assert(sizeof(FAT32_Directory_Entry) == 32, "wrong byte size!");
// static_assert(sizeof(FAT32_Directory_Entry) == sizeof(FAT32_Directory_Entry_LFN), "wrong byte size!");

#pragma pack(pop)

typedef struct {
    u32 attrib;
    u32 cluster;
    u32 size;
    u32 offset; // not used by any fat32_* funcs
} FAT32_File;

typedef u32 fat32_entry;

typedef struct {
    FAT32_Header header;
    u32 fat_size_in_sectors;
    u32 data_start_sector;
    fat32_entry *fat;
    u32 cluster_size; // in bytes
} FAT32_Volume;

typedef struct {
    u32 cluster;
    u32 entry; // in cluster

    u8 sector_buffer[SECTOR_SIZE];
    u32 buffered_sector;
} FAT32_DirList;

void fat32_init_volume(FAT32_Volume* volume);
bool fat32_find_file(FAT32_Volume* volume, const char* path, FAT32_File* out_file);
void fat32_read_file(FAT32_Volume* volume, FAT32_File* file, u8* out_buffer, u32 num_bytes, u32 start_offset);
void fat32_list_dir(FAT32_Volume* volume, FAT32_File* dir, FAT32_DirList* dir_list);
bool fat32_next_dir_entry(FAT32_Volume* volume, FAT32_DirList* dir_list, FAT32_File* out_file, char out_name[256]);
