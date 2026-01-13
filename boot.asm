[BITS 32]

section .multiboot
align 8
multiboot_header:
    dd 0xe85250d6              ; Multiboot2 magic
    dd 0
    dd header_end - multiboot_header
    dd -(0xe85250d6 + 0 + (header_end - multiboot_header))
    dw 0
    dw 0
    dd 8
header_end:

section .text
global start
extern kernel_main

start:
    cli
    mov esp, stack_top

    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Load PML4 table
    mov eax, pml4_table
    mov cr3, eax

    ; Enable Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable Paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64.pointer]

    ; Far jump to Long Mode
    jmp gdt64.code:long_mode_start


[BITS 64]
long_mode_start:
    mov ax, gdt64.data
    mov ds, ax
    mov es, ax
    mov ss, ax

    call kernel_main

.hang:
    hlt
    jmp .hang


; ================= GDT 64-bit =================

gdt64:
    dq 0

.code equ $ - gdt64
    dq 0x00AF9A000000FFFF    ; 64-bit code segment

.data equ $ - gdt64
    dq 0x00AF92000000FFFF    ; data segment

.pointer:
    dw $ - gdt64 - 1
    dq gdt64


; ================= Paging =================

align 4096
pml4_table:
    dq pdpt_table + 0x03      ; Present + Writable

align 4096
pdpt_table:
    dq pd_table + 0x03        ; Present + Writable

align 4096
pd_table:
    dq 0x0000000000000083    ; 2MB page: Present + Writable + PS
    times 511 dq 0


; ================= Stack =================

section .bss
align 16
stack_bottom:
    resb 8192
stack_top:
