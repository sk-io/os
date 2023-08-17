#include "elf.h"

#include "util.h"
#include "log.h"
#include "memory.h"
#include "physalloc.h"

// verify the magic identifer matches
bool elf_is_valid(Elf32_Ehdr* hdr) {
    return hdr->e_ident[0] == 0x7f && hdr->e_ident[1] == 'E' &&
        hdr->e_ident[2] == 'L' && hdr->e_ident[3] == 'F';
}

// copies ELF segments into their desired virtual addresses
u32 load_elf_segments(u8* elf) {
    Elf32_Ehdr* header = (Elf32_Ehdr*) elf;
    assert_msg(elf_is_valid(header), "elf identifier invalid");

    u32 num_segments = header->e_phnum;
    for (u32 i = 0; i < num_segments; i++) {
        Elf32_Phdr* segment = (Elf32_Phdr*) (elf + header->e_phoff + i * header->e_phentsize);
        
        // only copy segments with the LOAD flag
        if (segment->p_type != PT_LOAD)
            continue;

        u32 flags = PAGE_FLAG_USER;
        if (segment->p_flags & PF_W)
            flags |= PAGE_FLAG_WRITE;
        
        // allocate virtual pages
        u32 num_pages = CEIL_DIV(segment->p_memsz, 0x1000);
        for (int j = 0; j < num_pages; j++) {
            mem_map_page((segment->p_vaddr & ~0xFFF) + j * 0x1000, pmm_alloc_pageframe(), flags);
        }

        // copy it.
        memcpy(segment->p_vaddr, elf + segment->p_offset, segment->p_filesz);

        // zero remaining bytes
        u32 file_start = (segment->p_vaddr & ~0xFFF) + segment->p_filesz;
        u32 file_end = (segment->p_vaddr & ~0xFFF) + num_pages * 0x1000;

        // kernel_log("ELF: zeroed %d bytes", file_end - file_start);
        memset(file_start, 0, file_end - file_start);
    }

    return header->e_entry;
}
