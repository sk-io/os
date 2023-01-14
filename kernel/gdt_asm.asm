
global flush_gdt
flush_gdt:
    mov eax, [esp + 4]
    lgdt [eax]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush_cs
flush_cs:
    ret

global flush_tss
flush_tss:
    mov ax, 0x28
    ltr ax
    ret