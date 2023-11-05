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

void find_and_load_shared_lib(const char* name) {
    kernel_log("attempting to load shared lib: %s", name);

    char path[MAX_PATH_LENGTH];
    snprintf(path, MAX_PATH_LENGTH, "/%s", name);
    kernel_log("path: %s", path);

    // try to match path
    SharedLibrary* lib = NULL;
    for (int i = 0; i < MAX_SHARED_LIBRARIES; i++) {
        if (loaded_shared_libs[i].state == 1) {
            if (strncmp(name, loaded_shared_libs[i].path, MAX_PATH_LENGTH) == 0) {
                lib = loaded_shared_libs + i;
                break;
            }
        }
    }

    if (lib == NULL) {
        // can't find it, load it from disk
        kernel_log("loading from disk..");
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

    // map it into userspace
    // load segments with offset

    // should we only load .text?

    int slib_slot = -1;
    for (int i = 0; i < MAX_SHARED_LIBS_PER_TASK; i++) {
        if (current_task->slibs[i].slib == NULL) {
            slib_slot = i;
            break;
        }
    }
    assert(slib_slot != -1);

    // ---------------------------- TEMP CODE ----------------------------
    OpenSharedLibrary* open_slib = &current_task->slibs[slib_slot];
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

    // resolve the refs

    lib->num_users++;
}
