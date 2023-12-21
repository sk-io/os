#include "fat32.h"

#include "util.h"
#include "disk.h"
#include "malloc.h"
#include "log.h"

#define DIR_ENTRY_ATTRIB_LFN 0x0F
#define DIR_ENTRY_ATTRIB_SUBDIR 0x10

const u32 sector_size = 512;

static u32 cluster_to_sector(const FAT32_Volume* volume, u32 cluster) {
	return volume->data_start_sector + (cluster - 2) * volume->header.sectors_per_cluster;
}

void fat32_read_file(FAT32_Volume* volume, FAT32_File* file, u8* out_buffer, u32 num_bytes, u32 start_offset) {
	FAT32_Header* header = &volume->header;

	// printf("fat32_read_file start=%u num=%u attrib=%02x\n", start_offset, num_bytes, file->attrib);
	if (start_offset + num_bytes > file->size && !(file->attrib & DIR_ENTRY_ATTRIB_SUBDIR)) {
		assert(false);
	}

	u32 cluster = file->cluster;
	// printf("advancing %u clusters\n", start_offset / volume->cluster_size);
	for (int i = 0; i < start_offset / volume->cluster_size; i++) {
		start_offset -= volume->cluster_size;
		cluster = volume->fat[cluster] & 0xFFFFFF;
		if (cluster == 0xFFFFFF) {
			break;
		}
	}

	assert(cluster != 0xFFFFFF);

	u32 bytes_left = num_bytes - start_offset;
	while (true) {
		for (int i = start_offset / sector_size; i < header->sectors_per_cluster; i++) {
			u8 buffer[sector_size];
			disk_read_sector(buffer, cluster_to_sector(volume, cluster) + i);

			u32 bytes_to_read = bytes_left > sector_size ? sector_size : bytes_left;
			bytes_to_read -= start_offset;
			memcpy(out_buffer, buffer + start_offset, bytes_to_read);
			bytes_left -= bytes_to_read;
			out_buffer += bytes_to_read;

			start_offset = 0;
		}

		cluster = volume->fat[cluster] & 0xFFFFFF;
		if (cluster == 0xFFFFFF)
			break;
	}
}

static void parse_short_filename(char output[13], FAT32_Directory_Entry* entry) {
	strncpy(output, (const char*)entry->short_name, 8);
	int i;
	for (i = 7; i >= 0; i--) {
		if (output[i] != ' ') {
			break;
		}
		output[i] = '\0';
	}

	if (strncmp((char*) entry->short_ext, "   ", 3) == 0) {
		output[i + 1] = '\0';
		return;
	}

	output[i + 1] = '.';
	output[i + 2] = '\0';
	strncat(output, (const char*)entry->short_ext, 3);
}

static char ucs2_to_ascii(u16 ucs2) {
	return ucs2 & 0xFF;
}

static FAT32_Directory_Entry* parse_lfn(FAT32_Volume* volume, FAT32_Directory_Entry_LFN* lfn_start, char output[256]) {
	assert(lfn_start->sequence & 0x40);
	const int chars_per_entry = 13;

	FAT32_Directory_Entry_LFN* entry = lfn_start;
	while (true) {
		//printf("   %02x\n", entry->sequence);
		if (entry->attrib != DIR_ENTRY_ATTRIB_LFN) // all LFN entries have attrib = 0x0F
			break; // we're done

		if (entry->sequence == 0xE5) { // deleted entry, ignore
			entry++;
			continue;
		}

		// collect split string into single array
		u16 collect[chars_per_entry];
		for (int i = 0; i < 5; i++)
			collect[i] = entry->name0[i];
		for (int i = 0; i < 6; i++)
			collect[5 + i] = entry->name1[i];
		for (int i = 0; i < 2; i++)
			collect[11 + i] = entry->name2[i];
		
		// entries are in reverse order
		u32 part = (entry->sequence & 0xF) - 1;

		for (int i = 0; i < chars_per_entry; i++) {
			output[part * chars_per_entry + i] = ucs2_to_ascii(collect[i]);
			if (collect == 0) // end of string
				break;
		}

		entry++;
	}

	return (FAT32_Directory_Entry*) entry;
}

// returns length in clusters
static u32 count_fat_chain_length(const FAT32_Volume* volume, u32 cluster) {
	u32 length = 1;

	while (true) {
		cluster = volume->fat[cluster] & 0xFFFFFF;

		if (cluster == 0xFFFFFF)
			break;

		length++;
	}

	return length;
}

static int toupper(int c) {
	return (c >= 'a' && c <= 'z') ? (c & ~32) : c;
}

static u32 fat32_strncmp_nocase(const char* s1, const char* s2, u32 n) {
    register unsigned char u1, u2;

    while (n-- > 0) {
        u1 = toupper((unsigned char) *s1++);
        u2 = toupper((unsigned char) *s2++);
        if (u1 != u2) return u1 - u2;
        if (u1 == '\0') return 0;
    }
    return 0;
}

// not recursive
static bool fat32_find_file_in_dir(FAT32_Volume* volume, FAT32_File* dir_file, const char* name, u32 name_len, FAT32_File* out_file) {
	// printf("find_file_in_dir %.*s\n", name_len, name);

	u32 dir_content_size_sectors = count_fat_chain_length(volume, dir_file->cluster);
	// printf("dir_content_size_sectors: %u\n", dir_content_size_sectors);

	u32 contents_size = dir_content_size_sectors * sector_size;
	u8* dir_contents = (u8*) kmalloc(contents_size);
	fat32_read_file(volume, dir_file, dir_contents, contents_size, 0);

	char entry_name[256];
	bool next_is_lfn = false;

	FAT32_Directory_Entry* found = NULL;
	FAT32_Directory_Entry* entry = (FAT32_Directory_Entry*)(dir_contents);
	while (true) {
		// printf(" entry sequence: %02x\n", entry->attrib);

		if (entry->short_name[0] == 0) {
			break; // last entry, we're done
		}
		
		if (entry->short_name[0] == 0x2E) {
			// . or .. entry
			entry++;
			continue;
		}

		if (entry->attrib == DIR_ENTRY_ATTRIB_LFN) { // LFN
			entry = parse_lfn(volume, (FAT32_Directory_Entry_LFN*) entry, entry_name);

			next_is_lfn = true;
			continue;
		} else if (!next_is_lfn) {
			parse_short_filename(entry_name, entry);
		}

		// printf("matching against %s\n", entry_name);
		if (fat32_strncmp_nocase(entry_name, name, name_len) == 0) {
			found = entry;
			break;
		}

		if (next_is_lfn)
			next_is_lfn = false;

		entry++;
	}

	if (found != NULL) {
		out_file->attrib = found->attrib;
		out_file->cluster = found->cluster_low | (u32) found->cluster_high << 16;
		out_file->size = found->file_size;
	} else {
		memset(out_file, 0, sizeof(FAT32_File));
	}

	kfree(dir_contents);
	return found != NULL;
}

void fat32_init_volume(FAT32_Volume* volume) {
	u8 sector_buffer[sector_size];
	disk_read_sector(sector_buffer, 0);
	memcpy(&volume->header, sector_buffer, sizeof(FAT32_Header));

	const FAT32_Header* header = &volume->header;

	if (header->boot_signature != 0x28 && header->boot_signature != 0x29) {
        assert_msg(0, "not valid FAT32!\n");
		return;
	}

	assert(header->bytes_per_sector == sector_size);

	volume->fat_size_in_sectors = header->table_size_16 == 0 ? header->table_size_32 : header->table_size_16;
	volume->data_start_sector = header->reserved_sector_count + header->table_count * volume->fat_size_in_sectors;

	volume->fat = (fat32_entry*) kmalloc(volume->fat_size_in_sectors * sector_size);

	for (int i = 0; i < volume->fat_size_in_sectors; i++) {
		disk_read_sector(sector_buffer, header->reserved_sector_count + i);
		memcpy(((u8*) volume->fat) + i * sector_size, sector_buffer, sector_size);
	}

	volume->cluster_size = volume->header.sectors_per_cluster * sector_size;
}

bool fat32_find_file(FAT32_Volume* volume, const char* path, FAT32_File* out_file) {
	u32 path_len = strlen(path);
	if (path_len == 0)
		return false;
	
	assert(path[0] == '/');

	// start at root dir
	FAT32_File file = {0};
	file.attrib |= DIR_ENTRY_ATTRIB_SUBDIR;
	file.cluster = volume->header.root_cluster;
	
	u32 start = 1;
	for (int i = 1; i < path_len; i++) {
		const char* name = path + start;

		if (path[i] == '/') {
			u32 name_len = i - start;
			// printf("---> try to enter subdir: %.*s\n", name_len, name);

			if (fat32_find_file_in_dir(volume, &file, name, name_len, &file)) {
				assert(file.attrib & DIR_ENTRY_ATTRIB_SUBDIR);
			} else {
				assert(false);
				return false;
			}

			start = i + 1;
		} else if (i == path_len - 1) {
			u32 name_len = i - start + 1;
			// printf("look for file: %.*s\n", name_len, name);

			if (fat32_find_file_in_dir(volume, &file, name, name_len, &file)) {
				*out_file = file;
				return true;
			} else {
				return false;
			}

			break;
		}
	}

	return false;
}
