#include "slib.h"

#include "util.h"
#include "printf.h"
#include "fatfs/fatfs_ff.h"
#include "elf.h"
#include "memory.h"
#include "tasks.h"

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

static void find_and_load_shared_lib(const char* name, Task* task) {
    kernel_log("attempting to load shared lib: %s", name);

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

    if (lib == NULL) {
        // can't find it, load it from disk
        kernel_log("shared lib is not loaded. loading from disk..");
        int index = find_slot();

        lib = loaded_shared_libs + index;
        lib->state = 1;
        lib->num_users = 0;
        strncpy(lib->path, path, MAX_PATH_LENGTH);
        
        FIL file;

        FRESULT res;
        res = f_open(&file, path, FA_READ);
        if (res != FR_OK) {
            kernel_log("find_and_load_shared_lib: failed to open executable file %s. error=%u", path, res);
            return;
        }

        ELFObject elf;
        elf.size = f_size(&file);
        if (elf.size < 52) {
            kernel_log("find_and_load_shared_lib: elf file is too small. size=%u", elf.size);
            f_close(&file);
            return;
        }

        elf.raw = kmalloc(elf.size);
        UINT br;
        res = f_read(&file, elf.raw, elf.size, &br);
        f_close(&file);

        if (res != FR_OK) {
            kfree(elf);
            kernel_log("find_and_load_shared_lib: failed to read from executable file %s. error=%u", path, res);
        }

        if (!parse_elf(&elf)) {
            kernel_log("find_and_load_shared_lib: failed to parse ELF binary");
            return;
        }

        lib->elf = elf; // copy
    }

    // kernel_log("task = %x", task);
    // map it into userspace
    // load segments with offset

    int slib_slot = -1;
    for (int i = 0; i < MAX_SHARED_LIBS_PER_TASK; i++) {
        if (task->slibs[i].slib == NULL) {
            slib_slot = i;
            break;
        }
    }
    assert(slib_slot != -1);
    assert(slib_slot == 0); // temporary

    // kernel_log("slib_slot = %d", slib_slot);

    OpenSharedLibrary* open_slib = &task->slibs[slib_slot];
    open_slib->slib = lib;
    open_slib->offset = USER_SHARED_LIBS; // <------------------------------------ TODO

    Elf32_Ehdr* header = (Elf32_Ehdr*) lib->elf.header;
    u32 num_segments = header->e_phnum;
    for (u32 i = 0; i < num_segments; i++) {
        Elf32_Phdr* segment = (Elf32_Phdr*) (lib->elf.raw + header->e_phoff + i * header->e_phentsize);
        
        // only copy segments with the LOAD flag
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

        // TODO: don't copy, map virt pages in userspace to phys pages retrieved from
        // kmalloc'ed elf

        // allocate virtual pages
        for (int j = 0; j < num_pages; j++) {
            mem_map_page((addr & ~0xFFF) + j * 0x1000, pmm_alloc_pageframe(), flags);
            // kernel_log("mapping %x", (addr & ~0xFFF) + j * 0x1000);
        }

        // zero them, small optimization: dont zero to-be overwritten parts
        memset((addr & ~0xFFF), 0, num_pages * 0x1000);

        // copy it.
        memcpy(addr, lib->elf.raw + segment->p_offset, segment->p_filesz);
    }

    lib->num_users++;
}

static void resolve_dynamic_references(ELFObject* elf, Task* task) {
    // parse_symbol_table(elf, elf->dynamic_symbol_table, elf->dynamic_symbol_string_table);

    Elf32_Rel* relocation_table = elf->raw + elf->relocation_section->sh_offset;
    // kernel_log("size=%u, entsize=%u", elf->got_section->sh_size, elf->got_section->sh_entsize);

    u32* got = elf->got_section->sh_addr;

    // kernel_log("GOT at %x", got);
    u32 num_got_entries = elf->got_section->sh_size / elf->got_section->sh_entsize;
    for (int i = 3; i < num_got_entries; i++) {
        // kernel_log("  got[%d] = %x", i, &got[i]);
        int rel_index = i - 3;

        assert(ELF32_R_TYPE(relocation_table[rel_index].r_info) == R_386_JUMP_SLOT);
        u32 sym_index = ELF32_R_SYM(relocation_table[rel_index].r_info);
        const char* sym_name = get_elf_symbol(elf, elf->dynamic_symbol_table, elf->dynamic_symbol_string_table, sym_index);
        // kernel_log("   finding %s", sym_name);

        bool found = false;
        for (int s = 0; s < MAX_SHARED_LIBS_PER_TASK; s++) {
            OpenSharedLibrary* slib = &task->slibs[s];
            if (slib->slib == NULL)
                continue;
            
            ELFObject* slib_elf = &slib->slib->elf;

            Elf32_Sym* found_sym = find_symbol(slib_elf, slib_elf->dynamic_symbol_table, slib_elf->dynamic_symbol_string_table, sym_name);
            if (found_sym != NULL) {
                // kernel_log("    FOUND IT! addr=%x", found_sym->st_value);
                u32 addr = found_sym->st_value + slib->offset;
                // kernel_log("addr=%x", addr);
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
        
        find_and_load_shared_lib(name, task);
    }

    kernel_log("resolving dynamic references");
    resolve_dynamic_references(elf, task);
}
