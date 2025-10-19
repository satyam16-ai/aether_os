; syscall_handler.asm - System Call Handler (INT 0x80)
; This is called when user mode programs execute INT 0x80

extern syscall_handler

global syscall_wrapper

syscall_wrapper:
    ; Save all registers (build registers_t struct on stack)
    push ds
    push es
    push fs
    push gs
    
    pushad              ; Push all general purpose registers
    
    ; Load kernel data segment
    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler with pointer to registers
    push esp            ; Pass pointer to registers_t struct
    call syscall_handler
    add esp, 4          ; Clean up argument
    
    ; Restore all registers
    popad               ; Pop general purpose registers
    
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Return from interrupt (back to user mode)
    iret
