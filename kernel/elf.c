#include "elf.h"

#include "util.h"
#include "log.h"
#include "memory.h"
#include "physalloc.h"
#include "slib.h"
#include "tasks.h"

static void parse_dynamic_section(ELFObject* elf, Elf32_Shdr* section);
static void parse_symbol_table(ELFObject* elf, Elf32_Shdr* section, Elf32_Shdr* string_table);
static const char* get_string(const ELFObject* elf, u32 str_offset, Elf32_Shdr* string_table_section);
Elf32_Sym* find_symbol(ELFObject* elf, Elf32_Shdr* symtab_section, Elf32_Shdr* string_table, const char* symbol);

typedef struct {
    const char** table;
    u32 num_entries;
} SymbolTable;

// verify the magic identifer matches
bool elf_is_valid(Elf32_Ehdr* hdr) {
    return hdr->e_ident[0] == 0x7f && hdr->e_ident[1] == 'E' &&
        hdr->e_ident[2] == 'L' && hdr->e_ident[3] == 'F';
}

static Elf32_Shdr* get_section(const ELFObject* elf, u32 index) {
    return (Elf32_Shdr*) (elf->raw + elf->header->e_shoff + index * elf->header->e_shentsize);
}

static const char* get_section_name(const ELFObject* elf, Elf32_Shdr* section) {
    Elf32_Shdr* shstrtab_section = get_section(elf, elf->header->e_shstrndx);
    return get_string(elf, section->sh_name, shstrtab_section);
}

static const char* get_string(const ELFObject* elf, u32 str_offset, Elf32_Shdr* string_table_section) {
    const char* string_table = elf->raw + string_table_section->sh_offset;
    return string_table + str_offset;
}

const char* get_elf_symbol(const ELFObject* elf, Elf32_Shdr* symtab, Elf32_Shdr* strtab, int index) {
    Elf32_Sym* table = elf->raw + symtab->sh_offset;
    return get_string(elf, table[index].st_name, strtab);
}

// requires elf to be mem mapped?
void parse_dynamic_section(ELFObject* elf, Elf32_Shdr* section) {
    Elf32_Dyn* table = elf->raw + section->sh_offset;
    // kernel_log("parsing .dynamic section");
    int num_entries = section->sh_size / section->sh_entsize;
    // kernel_log("num .dynamic entries = %u", num_entries);

    // find DT_STRTAB
    Elf32_Dyn* strtab = NULL;
    for (int i = 0; i < num_entries; i++) {
        if (table[i].d_tag == DT_STRTAB) {
            strtab = table + i;
            // kernel_log("found dynamic string table");
            break;
        }
    }
    assert(strtab != NULL);

    const char* string_table = strtab->d_un.d_ptr;
    // kernel_log("dynamic string table addr = %x", strtab->d_un.d_ptr);

    int num = 0;
    for (int i = 0; i < num_entries; i++) {
        // kernel_log("dynamic entry: %d", table[i].d_tag);

        if (table[i].d_tag == DT_NEEDED) {
            const char* needed = string_table + table[i].d_un.d_val;
            elf->dynamic_dependencies[num] = needed;
            num++;
        }
    }
}

void parse_symbol_table(ELFObject* elf, Elf32_Shdr* section, Elf32_Shdr* string_table) {
    // kernel_log("parsing symbol table");

    Elf32_Sym* table = elf->raw + section->sh_offset;
    u32 num_entries = section->sh_size / section->sh_entsize;

    for (int i = 0; i < num_entries; i++) {
        const char* name = get_string(elf, table[i].st_name, string_table);
        kernel_log("symbol: %s", name);
    }
}

bool parse_elf(ELFObject* elf) {
    elf->header = (Elf32_Ehdr*) elf->raw;

    if (!elf_is_valid(elf->header)) {
        kernel_log("elf identifier invalid");
        return false;
    }

    u32 num_sections = elf->header->e_shnum;
    // kernel_log("ELF: num sections %u", num_sections);
    for (u32 i = 0; i < num_sections; i++) {
        Elf32_Shdr* section = get_section(elf, i);
        const char* name = get_section_name(elf, section);
        // kernel_log("parsing section %s: type=%u size=%u", name, section->sh_type, section->sh_size);

        if (strcmp(name, ".dynamic") == 0) {
            elf->dynamic_section = section;
        } else if (strcmp(name, ".dynsym") == 0) {
            elf->dynamic_symbol_table = section;
        } else if (strcmp(name, ".dynstr") == 0) {
            elf->dynamic_symbol_string_table = section;
        } else if (strcmp(name, ".got.plt") == 0) {
            elf->got_section = section;
        } else if (strcmp(name, ".rel.plt") == 0) {
            elf->relocation_section = section;
        }
    }

    if (elf->dynamic_section != NULL) {
        parse_dynamic_section(elf, elf->dynamic_section);
    }

    return true;
}

bool load_elf_executable(ELFObject* elf) {
    assert(elf->header);

    if (elf->header->e_type != 0x02) {
        kernel_log("elf is not an executable");
        return false;
    }

    elf->entry = elf->header->e_entry;

    u32 num_segments = elf->header->e_phnum;
    for (u32 i = 0; i < num_segments; i++) {
        Elf32_Phdr* segment = (Elf32_Phdr*) (elf->raw + elf->header->e_phoff + i * elf->header->e_phentsize);
        
        // only copy segments with the LOAD flag
        if (segment->p_type != PT_LOAD)
            continue;
        
        if (segment->p_memsz == 0)
            continue;
        
        u32 flags = PAGE_FLAG_USER;
        if (segment->p_flags & PF_W)
            flags |= PAGE_FLAG_WRITE;
        // TODO: executable??
        
        u32 num_pages = CEIL_DIV(segment->p_memsz, 0x1000);
        if (segment->p_vaddr & 0xFFF) {
            // if vaddr is not page aligned we need to map the next one aswell
            num_pages++;
        }

        // allocate virtual pages
        for (int j = 0; j < num_pages; j++) {
            mem_map_page((segment->p_vaddr & ~0xFFF) + j * 0x1000, pmm_alloc_pageframe(), flags);
        }
        // zero them, small optimization: dont zero to-be overwritten parts
        memset((segment->p_vaddr & ~0xFFF), 0, num_pages * 0x1000);

        // copy it.
        memcpy(segment->p_vaddr, elf->raw + segment->p_offset, segment->p_filesz);
    }

    if (elf->dynamic_section == NULL) {
        // statically linked, we're done
        kernel_log("executable is statically linked");
        return true;
    }

    return true;
}

// TODO: optimize
Elf32_Sym* find_symbol(ELFObject* elf, Elf32_Shdr* symtab_section, Elf32_Shdr* string_table, const char* symbol) {
    // kernel_log("parsing symbol table");

    Elf32_Sym* table = elf->raw + symtab_section->sh_offset;
    u32 num_entries = symtab_section->sh_size / symtab_section->sh_entsize;

    for (int i = 0; i < num_entries; i++) {
        const char* name = get_string(elf, table[i].st_name, string_table);
        // kernel_log("symbol: %s", name);

        if (strcmp(name, symbol) == 0)
            return table + i;
    }

    return NULL;
}
