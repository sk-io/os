#include "slib.h"

#include "util.h"
#include "printf.h"
#include "elf.h"
#include "memory.h"
#include "tasks.h"
#include "disk.h"

SharedLibrary loaded_shared_libs[MAX_SHARED_LIBRARIES];

void init_shared_libs() {
    memset(loaded_shared_libs, 0, sizeof(loaded_shared_libs));
}

static int find_slot() {
    for (int i = 0; i < MAX_SHARED_LIBRARIES; i++) {
        if (loaded_shared_libs[i].state == 0) {
            return i;
        }
    }
    assert_msg(0, "too many shared libraries!");
    return -1;
}

static SharedLibrary* find_or_load_shared_lib(const char* name) {
    char path[MAX_PATH_LENGTH];
    snprintf(path, MAX_PATH_LENGTH, "/%s", name);

    // todo: multiple search paths

    // try to match path
    SharedLibrary* lib = NULL;
    for (int i = 0; i < MAX_SHARED_LIBRARIES; i++) {
        if (loaded_shared_libs[i].state == 1) {
            
            if (strncmp(path, loaded_shared_libs[i].path, MAX_PATH_LENGTH) == 0) {
                lib = loaded_shared_libs + i;
                break;
            }
        }
    }

    // found it
    if (lib != NULL)
        return lib;

    // can't find it, load it from disk
    kernel_log("loading %s", path);
    int index = find_slot();

    lib = loaded_shared_libs + index;
    lib->state = 1;
    lib->num_users = 0;
    strncpy(lib->path, path, MAX_PATH_LENGTH);
    
    FAT32_File file;
    if (!fat32_find_file(&ramdisk.volume, path, &file)) {
        kernel_log("find_and_load_shared_lib: failed to open library %s", path);
        return NULL;
    }

    ELFObject elf;
    elf.size = file.size;
    if (elf.size < 52) {
        kernel_log("find_and_load_shared_lib: elf file is too small. size=%u", elf.size);
        return NULL;
    }

    // hack: pad malloc until page aligned.
    // FIXME: memleak
    elf.mem = kmalloc(elf.size + 0x1000);

    elf.raw = elf.mem;
    if ((u32) elf.raw & 0xFFF) {
        u32 addr = (u32) elf.raw;
        addr &= ~0xFFF;
        addr += 0x1000;
        elf.raw = (u8*) addr;
    }

    fat32_read_file(&ramdisk.volume, &file, elf.raw, file.size, 0);

    // if (res != FR_OK) {
    //     kfree(elf.mem);
    //     kernel_log("find_and_load_shared_lib: failed to read from executable file %s. error=%u", path, res);
    //     return NULL;
    // }

    if (!parse_elf(&elf)) {
        kfree(elf.mem);
        kernel_log("find_and_load_shared_lib: failed to parse ELF binary");
        return NULL;
    }

    lib->elf = elf; // copy
    return lib;

    // TODO: use goto cleanup
}

// FIXME: acts on current pdir
static void map_shared_lib_into_task(SharedLibrary* lib, Task* task) {
    int slib_slot = -1;
    for (int i = 0; i < MAX_SHARED_LIBS_PER_TASK; i++) {
        if (task->slibs[i].slib == NULL) {
            slib_slot = i;
            break;
        }
    }
    assert(slib_slot != -1);

    OpenSharedLibrary* open_slib = &task->slibs[slib_slot];
    open_slib->slib = lib;
    open_slib->offset = USER_SHARED_LIBS;
    
    // load it immediately after the previous one
    if (slib_slot > 0) {
        OpenSharedLibrary* prev = &task->slibs[slib_slot - 1];
        open_slib->offset = USER_SHARED_LIBS + prev->offset + prev->slib->elf.size;
        
        // pad it upwards to nearest page
        if (open_slib->offset & 0xFFF) {
            open_slib->offset &= ~0xFFF;
            open_slib->offset += 0x1000;
        }
    }

    // read elf segments and map or copy them
    Elf32_Ehdr* header = (Elf32_Ehdr*) lib->elf.header;
    u32 num_segments = header->e_phnum;
    for (u32 i = 0; i < num_segments; i++) {
        Elf32_Phdr* segment = (Elf32_Phdr*) (lib->elf.raw + header->e_phoff + i * header->e_phentsize);
        
        if (segment->p_type != PT_LOAD)
            continue;
        
        if (segment->p_memsz == 0)
            continue;
        
        u32 flags = PAGE_FLAG_USER;
        if (segment->p_flags & PF_W)
            flags |= PAGE_FLAG_WRITE;
        // TODO: execute??

        u32 addr = segment->p_vaddr + open_slib->offset;
        
        u32 num_pages = CEIL_DIV(segment->p_memsz, 0x1000);
        if (addr & 0xFFF) {
            // if vaddr is not page aligned we need to map the next one aswell
            num_pages++;
        }

        if (segment->p_flags & PF_W) {
            // allocate virtual pages
            for (int j = 0; j < num_pages; j++) {
                mem_map_page((addr & ~0xFFF) + j * 0x1000, pmm_alloc_pageframe(), flags | PAGE_FLAG_OWNER);
            }

            // zero them, small optimization: dont zero to-be overwritten parts
            memset(addr & ~0xFFF, 0, num_pages * 0x1000);

            // copy data
            memcpy(addr, lib->elf.raw + segment->p_offset, segment->p_filesz);
        } else {
            // map to kmalloc'd physical pages
            for (int j = 0; j < num_pages; j++) {
                u32 phys = mem_get_phys_from_virt(lib->elf.raw + segment->p_offset + j * 0x1000);
                mem_map_page((addr & ~0xFFF) + j * 0x1000, phys, flags);
            }
        }
    }

    lib->num_users++;
}

static void resolve_dynamic_references(ELFObject* elf, Task* task) {
    Elf32_Rel* relocation_table = elf->raw + elf->relocation_section->sh_offset;

    u32* got = elf->got_section->sh_addr;
    u32 num_got_entries = elf->got_section->sh_size / elf->got_section->sh_entsize;

    for (int i = 3; i < num_got_entries; i++) {
        int rel_index = i - 3;

        assert(ELF32_R_TYPE(relocation_table[rel_index].r_info) == R_386_JUMP_SLOT);
        u32 sym_index = ELF32_R_SYM(relocation_table[rel_index].r_info);
        const char* sym_name = get_elf_symbol(elf, elf->dynamic_symbol_table, elf->dynamic_symbol_string_table, sym_index);

        bool found = false;
        for (int s = 0; s < MAX_SHARED_LIBS_PER_TASK; s++) {
            OpenSharedLibrary* slib = &task->slibs[s];
            if (slib->slib == NULL)
                continue;
            
            ELFObject* slib_elf = &slib->slib->elf;

            Elf32_Sym* found_sym = find_symbol(slib_elf, slib_elf->dynamic_symbol_table, slib_elf->dynamic_symbol_string_table, sym_name);
            if (found_sym != NULL) {
                u32 addr = found_sym->st_value + slib->offset;
                got[i] = addr;
                found = true;
                break;
            }
        }

        assert(found);
    }
}

void init_shared_libs_for_task(u32 task_id, ELFObject* elf) {
    Task* task = get_task(task_id);
    for (int i = 0; i < MAX_SHARED_LIBS_PER_TASK; i++) {
        const char* name = elf->dynamic_dependencies[i];
        if (name == NULL)
            continue;

        SharedLibrary* lib = find_or_load_shared_lib(name);
        map_shared_lib_into_task(lib, task);
    }

    // kernel_log("resolving dynamic references");
    // we need to do this last, after all the libs are loaded and mapped.
    resolve_dynamic_references(elf, task);
}
