#include "elf.h"

#include "util.h"
#include "log.h"
#include "memory.h"
#include "physalloc.h"

bool elf_is_valid(Elf32_Ehdr* hdr) {
    return hdr->e_ident[0] == 0x7f && hdr->e_ident[1] == 'E' &&
        hdr->e_ident[2] == 'L' && hdr->e_ident[3] == 'F';
}

u32 load_elf(u8* elf) {
    Elf32_Ehdr* header = (Elf32_Ehdr*) elf;
    assert_msg(elf_is_valid(header), "elf identifier invalid");

    // kernel_log("parsing elf");
    // kernel_log("entry=%x", header->e_entry);
    // kernel_log("num segments=%x", header->e_phnum);

    u32 num_segments = header->e_phnum;
    for (u32 i = 0; i < num_segments; i++) {
        Elf32_Phdr* segment = (Elf32_Phdr*) (elf + header->e_phoff + i * header->e_phentsize);
        
        if (segment->p_type != PT_LOAD)
            continue;

        // kernel_log("PT_LOAD segment no. %d", i);
        // kernel_log("  p_flags  =%x", segment->p_flags);
        // kernel_log("  p_vaddr  =%x", segment->p_vaddr);
        // kernel_log("  p_memsz  =%x", segment->p_memsz);
        // kernel_log("  p_filesz =%x", segment->p_filesz);
        
        u32 flags = PAGE_FLAG_USER;
        if (segment->p_flags & PF_W)
            flags |= PAGE_FLAG_WRITE;
        
        u32 num_pages = CEIL_DIV(segment->p_memsz, 0x1000);
        // kernel_log("  num_pages=%d", num_pages);
        for (int j = 0; j < num_pages; j++) {
            mem_map_page((segment->p_vaddr & ~0xFFF) + j * 0x1000, pmm_alloc_pageframe(), flags);
        }

        // kernel_log("load_elf memcpy vaddr=%x", segment->p_vaddr);
        memcpy(segment->p_vaddr, elf + segment->p_offset, segment->p_filesz);
    }

    return header->e_entry;
}
