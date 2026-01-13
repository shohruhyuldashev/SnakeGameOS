[BITS 64]
global timer_isr
extern timer_handler

; x86_64 interrupt handler stub
timer_isr:
    ; CPU already pushed:
    ; RIP, CS, RFLAGS, RSP, SS (because interrupt from ring0 with IST=0)
    ; We must preserve general registers manually

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    call timer_handler

    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al

    ; Restore registers
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq
