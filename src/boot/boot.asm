; WolfTech OS Bootloader - Enterprise Edition
; Animated boot sequence with company branding

BITS 16
ORG 0x7C00

section .text
    global _start

_start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Save boot drive
    mov [boot_drive], dl

    ; Clear screen and set video mode
    mov ax, 0x0003
    int 0x10

    ; Set up for animation
    call boot_animation

    ; Load kernel from disk (sectors 2-100 to 0x1000)
    mov bx, 0x1000
    mov dh, 98          ; Number of sectors to read (larger kernel)
    mov dl, [boot_drive]
    call load_kernel

    ; Enable A20 line
    call enable_a20

    ; Load GDT
    cli
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp CODE_SEG:protected_mode

; Boot animation with WolfTech branding
boot_animation:
    pusha
    
    ; Frame 1 - Loading dots
    mov si, frame1
    call print_string_centered
    call delay_short
    
    ; Frame 2 - More dots
    mov si, frame2
    call print_string_centered
    call delay_short
    
    ; Frame 3 - Company name appears
    mov si, frame3
    call print_string_centered
    call delay_long
    
    ; Frame 4 - Full branding
    call clear_screen
    mov si, logo_line1
    mov dh, 8
    call print_at_pos
    
    mov si, logo_line2
    mov dh, 9
    call print_at_pos
    
    mov si, logo_line3
    mov dh, 10
    call print_at_pos
    
    mov si, company_name
    mov dh, 12
    call print_at_pos
    
    mov si, tagline
    mov dh, 13
    call print_at_pos
    
    mov si, loading_msg
    mov dh, 20
    call print_at_pos
    
    call delay_long
    call delay_long
    
    popa
    ret

clear_screen:
    pusha
    mov ax, 0x0003
    int 0x10
    popa
    ret

print_at_pos:
    pusha
    mov ah, 0x02        ; Set cursor position
    mov bh, 0           ; Page 0
    mov dl, 0           ; Column 0 (will center)
    int 0x10
    call print_string_centered
    popa
    ret

print_string_centered:
    pusha
    ; Calculate length
    mov di, si
    xor cx, cx
.len_loop:
    lodsb
    or al, al
    jz .len_done
    inc cx
    jmp .len_loop
.len_done:
    mov si, di
    
    ; Center it (80 - len) / 2
    mov ax, 80
    sub ax, cx
    shr ax, 1
    mov dl, al
    mov ah, 0x02
    mov bh, 0
    int 0x10
    
    ; Print string
.print_loop:
    lodsb
    or al, al
    jz .print_done
    mov ah, 0x0E
    mov bl, 0x0F        ; White on black
    int 0x10
    jmp .print_loop
.print_done:
    popa
    ret

delay_short:
    pusha
    mov cx, 0x0001
    mov dx, 0x0000
    mov ah, 0x86
    int 0x15
    popa
    ret

delay_long:
    pusha
    mov cx, 0x0004
    mov dx, 0x0000
    mov ah, 0x86
    int 0x15
    popa
    ret

; Load kernel from disk
load_kernel:
    pusha
    push dx

    mov ah, 0x02        ; Read sectors
    mov al, dh          ; Number of sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2
    mov dh, 0           ; Head 0
    int 0x13

    jc disk_error
    
    pop dx
    cmp al, dh
    jne disk_error
    
    popa
    ret

disk_error:
    mov si, disk_error_msg
    call print_string_centered
    jmp $

; Enable A20 line using keyboard controller
enable_a20:
    call wait_8042
    mov al, 0xAD
    out 0x64, al

    call wait_8042
    mov al, 0xD0
    out 0x64, al

    call wait_8042_data
    in al, 0x60
    push ax

    call wait_8042
    mov al, 0xD1
    out 0x64, al

    call wait_8042
    pop ax
    or al, 2
    out 0x60, al

    call wait_8042
    mov al, 0xAE
    out 0x64, al

    call wait_8042
    ret

wait_8042:
    in al, 0x64
    test al, 2
    jnz wait_8042
    ret

wait_8042_data:
    in al, 0x64
    test al, 1
    jz wait_8042_data
    ret

BITS 32
protected_mode:
    ; Set up segments for 32-bit
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Jump to kernel
    jmp 0x1000

; GDT
gdt_start:
    ; Null descriptor
    dq 0

gdt_code:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0

gdt_data:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Data
boot_drive: db 0

; Boot animation frames
frame1: db "Initializing...", 0
frame2: db "Initializing... Loading system", 0
frame3: db "Starting WolfTech OS", 0

; ASCII art logo
logo_line1: db "  _       __      ______   ______           __  ", 0
logo_line2: db " | |     / /___  / / __/__/_  __/__  _____/ /_ ", 0
logo_line3: db " | | /| / / __ \\/ / /_____/ / / _ \\/ ___/ __ \\", 0

company_name: db "          WOLFTECH INNOVATIONS          ", 0
tagline:      db "     Enterprise Server Operating System     ", 0
loading_msg:  db "           Loading kernel...           ", 0

disk_error_msg: db "Disk read error!", 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
