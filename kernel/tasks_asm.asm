extern current_task
extern tss

; void switch_context(Task* old, Task* new);
global switch_context
switch_context:
    mov eax, [esp + 4] ; eax = old
    mov edx, [esp + 8] ; edx = new

    ; push registers that arent already saved by cdecl call etc.
    push ebp
    push ebx
    push esi
    push edi

    ; swap kernel stack pointer
    mov [eax + 4], esp ; old->kesp = esp
    mov esp, [edx + 4] ; esp = new->kesp

    ; cmp dword [edx + 16], 1 ; is this a kernel task?
    ; je kernel_task          ; if so, skip changing page dirs
    ; PROBLEM: the current pagedir could be destroyed/invalid

    ; change page dir
    mov eax, [edx + 12]
    sub eax, 0xC0000000
    mov cr3, eax
; kernel_task:

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret ; new tasks change the return value using TaskReturnContext.eip