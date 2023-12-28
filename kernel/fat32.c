#include "fat32.h"

#include "util.h"
#include "disk.h"
#include "malloc.h"
#include "log.h"

#define DIR_ENTRY_ATTRIB_LFN 0x0F

static u32 cluster_to_sector(const FAT32_Volume* volume, u32 cluster) {
	return volume->data_start_sector + (cluster - 2) * volume->header.sectors_per_cluster;
}

static int toupper(int c) {
	return (c >= 'a' && c <= 'z') ? (c & ~32) : c;
}

static int tolower(int c) {
	return (c >= 'A' && c <= 'Z') ? (c | 32) : c;
}

void fat32_read_file(FAT32_Volume* volume, FAT32_File* file, u8* out_buffer, u32 num_bytes, u32 start_offset) {
	FAT32_Header* header = &volume->header;

	// printf("fat32_read_file start=%u num=%u attrib=%02x\n", start_offset, num_bytes, file->attrib);
	if (start_offset + num_bytes > file->size && !(file->attrib & FAT32_IS_DIR)) {
		assert(false);
	}

	u32 cluster = file->cluster;
	u32 clusters_to_advance = start_offset / volume->cluster_size;
	// kernel_log("advancing %u clusters", clusters_to_advance);
	for (int i = 0; i < clusters_to_advance; i++) {
		start_offset -= volume->cluster_size;
		cluster = volume->fat[cluster] & 0xFFFFFF;
		assert(cluster != 0xFFFFFF);
	}

	assert(cluster != 0xFFFFFF);

	u32 bytes_left = num_bytes;
	while (true) {
		for (int i = start_offset / SECTOR_SIZE; i < header->sectors_per_cluster; i++) {
			start_offset %= SECTOR_SIZE;
			
			u8 buffer[SECTOR_SIZE];
			disk_read_sector(buffer, cluster_to_sector(volume, cluster) + i);

			u32 bytes_to_read = bytes_left;
			if (bytes_left + start_offset > SECTOR_SIZE) {
				// reaches into the next sector
				bytes_to_read = SECTOR_SIZE - start_offset;
			}

			memcpy(out_buffer, buffer + start_offset, bytes_to_read);
			bytes_left -= bytes_to_read;
			out_buffer += bytes_to_read;

			start_offset = 0;
			if (bytes_left == 0)
				return;
		}

		cluster = volume->fat[cluster] & 0xFFFFFF;
		if (cluster == 0xFFFFFF)
			break;
	}
}

// dos style 8.3
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

	if (entry->lowercase) {
		for (i = 0; i < 13; i++) {
			output[i] = tolower(output[i]);
		}
	}
}

static char ucs2_to_ascii(u16 ucs2) {
	return ucs2 & 0xFF;
}

#define LFN_CHARS_PER_ENTRY 13

static void parse_lfn_entry(FAT32_Volume* volume, FAT32_Directory_Entry_LFN* lfn_start, char output[256]) {
	//assert(lfn_start->sequence & 0x40);

	FAT32_Directory_Entry_LFN* entry = lfn_start;
		//printf("   %02x\n", entry->sequence);
	if (entry->attrib != DIR_ENTRY_ATTRIB_LFN) // all LFN entries have attrib = 0x0F
		return;

	if (entry->sequence == 0xE5) { // deleted entry, ignore
		return;
	}

	// collect split string into single array
	u16 collect[LFN_CHARS_PER_ENTRY];
	for (int i = 0; i < 5; i++)
		collect[i] = entry->name0[i];
	for (int i = 0; i < 6; i++)
		collect[5 + i] = entry->name1[i];
	for (int i = 0; i < 2; i++)
		collect[11 + i] = entry->name2[i];

	// entries are in reverse order
	u32 part = (entry->sequence & 0xF) - 1;

	for (int i = 0; i < LFN_CHARS_PER_ENTRY; i++) {
		output[part * LFN_CHARS_PER_ENTRY + i] = ucs2_to_ascii(collect[i]);
		if (collect == 0) // end of string
			break;
	}
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

static bool find_file_in_dir(FAT32_Volume* volume, FAT32_File* dir_file, const char* name, u32 name_len, FAT32_File* out_file) {
	FAT32_DirList list;
	fat32_list_dir(volume, dir_file, &list);

	char entry_name[256];
	while (fat32_next_dir_entry(volume, &list, out_file, entry_name)) {
		if (fat32_strncmp_nocase(entry_name, name, name_len) == 0) {
			return true;
		}
	}

	return false;
}

void fat32_init_volume(FAT32_Volume* volume) {
	u8 sector_buffer[SECTOR_SIZE];
	disk_read_sector(sector_buffer, 0);
	memcpy(&volume->header, sector_buffer, sizeof(FAT32_Header));

	const FAT32_Header* header = &volume->header;

	if (header->boot_signature != 0x28 && header->boot_signature != 0x29) {
        assert(0);
		return;
	}

	assert(header->bytes_per_sector == SECTOR_SIZE);

	volume->fat_size_in_sectors = header->table_size_16 == 0 ? header->table_size_32 : header->table_size_16;
	volume->data_start_sector = header->reserved_sector_count + header->table_count * volume->fat_size_in_sectors;

	volume->fat = (fat32_entry*) kmalloc(volume->fat_size_in_sectors * SECTOR_SIZE);

	for (int i = 0; i < volume->fat_size_in_sectors; i++) {
		disk_read_sector(sector_buffer, header->reserved_sector_count + i);
		memcpy(((u8*) volume->fat) + i * SECTOR_SIZE, sector_buffer, SECTOR_SIZE);
	}

	volume->cluster_size = volume->header.sectors_per_cluster * SECTOR_SIZE;

	printf("Initialized FAT32 filesystem\n");
	printf("[FAT32] cluster size=%u sector(s)\n", volume->header.sectors_per_cluster);
	printf("[FAT32] FAT size=%u bytes\n", volume->fat_size_in_sectors * SECTOR_SIZE);
}

bool fat32_find_file(FAT32_Volume* volume, const char* path, FAT32_File* out_file) {
	u32 path_len = strlen(path);
	if (path_len == 0)
		return false;

	// start at root dir
	FAT32_File file = {0};
	file.attrib |= FAT32_IS_DIR;
	file.cluster = volume->header.root_cluster;
	
	if (path[0] == '/' && path[1] == '\0') {
		// root dir "/"
		*out_file = file;
		return true;
	}

	u32 start = path[0] == '/' ? 1 : 0;
	for (int i = start; i < path_len; i++) {
		const char* name = path + start;

		if (path[i] == '/') {
			// enter subdirectory

			u32 name_len = i - start;
			if (!find_file_in_dir(volume, &file, name, name_len, &file)) {
				return false; // couldnt find subdir
			}

			assert(file.attrib & FAT32_IS_DIR);
			start = i + 1;
		} else if (i == path_len - 1) {
			// name of the file

			u32 name_len = i - start + 1;
			if (find_file_in_dir(volume, &file, name, name_len, &file)) {
				*out_file = file; // we found the file!
				return true;
			}

			return false;
		}
	}

	return false;
}

void fat32_list_dir(FAT32_Volume* volume, FAT32_File* dir, FAT32_DirList* dir_list) {
	assert(dir->attrib & FAT32_IS_DIR);

	dir_list->cluster = dir->cluster;
	dir_list->entry = 0;
	dir_list->buffered_sector = 0;
}

static FAT32_Directory_Entry* read_next_entry(FAT32_Volume* volume, FAT32_DirList* dir_list) {
	u32 entries_per_cluster = volume->cluster_size / sizeof(FAT32_Directory_Entry);
	u32 entries_per_sector = SECTOR_SIZE / sizeof(FAT32_Directory_Entry);

	if (dir_list->entry >= entries_per_cluster) {
		// advance cluster
		dir_list->cluster = volume->fat[dir_list->cluster] & 0xFFFFFF;
		assert(dir_list->cluster != 0xFFFFFF);
		dir_list->entry = 0;
	}

	u32 right_sector = cluster_to_sector(volume, dir_list->cluster) + dir_list->entry / entries_per_sector;
	if (dir_list->buffered_sector != right_sector) {
		// read next sector
		disk_read_sector(dir_list->sector_buffer, right_sector);
		dir_list->buffered_sector = right_sector;
	}
	
	// now we're in the right cluster and sector
	u32 entry_in_sector = dir_list->entry % entries_per_sector;
	FAT32_Directory_Entry* entry = dir_list->sector_buffer + entry_in_sector * sizeof(FAT32_Directory_Entry);
	dir_list->entry++;

	if (entry->short_name[0] == 0)
		return NULL; // end of list signifier

	return entry;
}

bool fat32_next_dir_entry(FAT32_Volume* volume, FAT32_DirList* dir_list, FAT32_File* out_file, char out_name[256]) {
	bool is_lfn = false;
	FAT32_Directory_Entry* entry;

	while (true) {
		// read entry
		entry = read_next_entry(volume, dir_list);
		if (entry == NULL)
			return false; // we hit the end
		if (entry->short_name[0] == 0x2E)
			continue; // skip "." and ".." entries
		
		if (entry->attrib == DIR_ENTRY_ATTRIB_LFN) {
			is_lfn = true;
			parse_lfn_entry(volume, (FAT32_Directory_Entry_LFN*)entry, out_name);
			continue;
		}

		// last entry of a LFN set is a normal entry
		if (!is_lfn) {
			parse_short_filename(out_name, entry);
		}

		// we found our entry
		break;
	}

	out_file->attrib = entry->attrib;
	out_file->cluster = entry->cluster_low | (u32)entry->cluster_high << 16;
	out_file->offset = 0;
	out_file->size = entry->file_size;
	return true;
}
