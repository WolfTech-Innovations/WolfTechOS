; WolfTech OS Kernel Entry
BITS 32

section .text
    global _start
    extern kernel_main
    extern handle_syscall
    
_start:
    ; Set up stack
    mov esp, kernel_stack_top
    
    ; Call kernel main
    call kernel_main
    
    ; Halt if kernel returns
.halt:
    cli
    hlt
    jmp .halt

; Interrupt Service Routines
global isr_timer
global isr_keyboard
global isr_syscall

extern timer_handler
extern keyboard_handler

; Timer ISR (IRQ0)
isr_timer:
    pusha
    call timer_handler
    mov al, 0x20
    out 0x20, al
    popa
    iret

; Keyboard ISR (IRQ1)
isr_keyboard:
    pusha
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popa
    iret

; System call ISR (INT 0x80 - Linux compatible!)
isr_syscall:
    ; Save all registers
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax
    
    ; Create syscall_regs_t structure on stack
    mov ebp, esp
    push ebp            ; Pass pointer to registers
    
    call handle_syscall
    
    ; Clean up stack
    add esp, 4
    
    ; Restore registers except eax (return value)
    add esp, 4          ; Skip eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    
    iret

section .bss
    resb 32768          ; 32KB stack
kernel_stack_top:
