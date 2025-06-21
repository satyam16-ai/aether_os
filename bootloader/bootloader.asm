[org 0x7C00]
bits 16

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, logo
.print:
    lodsb
    or al, al
    jz hang

    cmp al, 10
    jne .print_char
    call newline
    jmp .print

.print_char:
    mov ah, 0x0E
    int 0x10
    jmp .print

newline:
    push ax
    mov ah, 0x0E
    mov al, 13
    int 0x10
    mov al, 10
    int 0x10
    pop ax
    ret

hang:
    cli
    hlt

logo db "   ___  __ _____ _    ___  ____  ____ ",10
     db "  / _ |/ // / _ | |  / _ )/ __ \/ __/",10
     db " / __ / _  / __ | |_/ _  / /_/ /\ \  ",10
     db "/_/ |_/_//_/_/ |_|___//_/\____/___/  ",10
     db "          Aether OS Bootloader       ",10
     db 0

times 510 - ($ - $$) db 0
dw 0xAA55

