
MBOOT_PAGE_ALIGN    equ 1 << 0
MBOOT_MEM_INFO      equ 1 << 1
MBOOT_USE_GFX       equ 1 << 2

MBOOT_MAGIC    equ 0x1BADB002
MBOOT_FLAGS    equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO | MBOOT_USE_GFX
MBOOT_CHECKSUM equ -(MBOOT_MAGIC + MBOOT_FLAGS)

section .multiboot
align 4
    dd MBOOT_MAGIC
    dd MBOOT_FLAGS
    dd MBOOT_CHECKSUM
    ; Info about where to load us.
    ; Since we are using ELF, mboot reads from its header instead
    dd 0, 0, 0, 0, 0

    ; Graphics requests
    dd 0 ; 0 = linear graphics
    dd 800
    dd 600
    dd 32

; setup stack
section .bss
align 16
stack_bottom:
    resb 16384*8
stack_top:

section .boot
global _start
_start:
    mov eax, (initial_page_dir - 0xC0000000)
    mov cr3, eax

    mov ecx, cr4
    or ecx, 0x10
    mov cr4, ecx

    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    jmp higher_half

section .text
higher_half:
    ; protected mode
    ; interrupts and paging disabled

    mov esp, stack_top

    push ebx ; multiboot mem info pointer

    xor ebp, ebp ; set ebp to 0 as a stopping point for stack tracing
    extern kernel_main
    call kernel_main

halt:
    hlt
    jmp halt

section .data
align 4096
global initial_page_dir ; optimization: use bss and build in assembly
initial_page_dir:
    dd 10000011b ; initial 4mb identity map, unmapped later

    times 768-1 dd 0 ; padding

    ; hh kernel start, map 16 mb
    dd (0 << 22) | 10000011b ; 0xC0000000
    dd (1 << 22) | 10000011b
    dd (2 << 22) | 10000011b
    dd (3 << 22) | 10000011b
    times 256-4 dd 0 ; padding

    ; dd initial_page_dir | 11b
