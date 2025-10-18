; idt_flush.asm - Assembly routine to load IDT
; Also contains ISR stubs that save state and call C handlers

SECTION .text
GLOBAL idt_flush

; Load IDT function
idt_flush:
    mov eax, [esp + 4]  ; Get pointer to IDT descriptor from stack
    lidt [eax]          ; Load IDT
    ret                 ; Return to caller

; Macro to create ISR stub without error code
%macro ISR_NOERRCODE 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        push byte 0         ; Push dummy error code
        push byte %1        ; Push interrupt number
        jmp isr_common_stub ; Jump to common handler
%endmacro

; Macro to create ISR stub with error code (CPU pushes error code automatically)
%macro ISR_ERRCODE 1
    global isr%1
    isr%1:
        cli                 ; Disable interrupts
        push byte %1        ; Push interrupt number
        jmp isr_common_stub ; Jump to common handler
%endmacro

; CPU Exception handlers (0-31)
ISR_NOERRCODE 0     ; Division by Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non-Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault (has error code)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun (legacy)
ISR_ERRCODE   10    ; Invalid TSS (has error code)
ISR_ERRCODE   11    ; Segment Not Present (has error code)
ISR_ERRCODE   12    ; Stack-Segment Fault (has error code)
ISR_ERRCODE   13    ; General Protection Fault (has error code)
ISR_ERRCODE   14    ; Page Fault (has error code)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 Floating-Point Exception
ISR_ERRCODE   17    ; Alignment Check (has error code)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception (has error code)
ISR_NOERRCODE 31    ; Reserved

; External C function to handle interrupts
extern isr_handler
extern irq_handler

; Macro to create IRQ stub
%macro IRQ 2
    global irq%1
    irq%1:
        cli
        push byte 0         ; Push dummy error code
        push byte %2        ; Push IRQ number
        jmp irq_common_stub ; Jump to common IRQ handler
%endmacro

; IRQ handlers (32-47 correspond to IRQ 0-15)
IRQ 0, 32   ; Timer
IRQ 1, 33   ; Keyboard
IRQ 2, 34   ; Cascade (never raised)
IRQ 3, 35   ; COM2
IRQ 4, 36   ; COM1
IRQ 5, 37   ; LPT2
IRQ 6, 38   ; Floppy
IRQ 7, 39   ; LPT1
IRQ 8, 40   ; CMOS Real-time clock
IRQ 9, 41   ; Free for peripherals
IRQ 10, 42  ; Free for peripherals
IRQ 11, 43  ; Free for peripherals
IRQ 12, 44  ; PS2 Mouse
IRQ 13, 45  ; FPU / Coprocessor
IRQ 14, 46  ; Primary ATA Hard Disk
IRQ 15, 47  ; Secondary ATA Hard Disk

; Common IRQ stub - saves all registers and calls C handler
irq_common_stub:
    ; Save all general-purpose registers
    pusha
    
    ; Save segment registers
    mov ax, ds
    push eax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Pass stack pointer as argument (pointer to registers_t structure)
    push esp
    call irq_handler
    add esp, 4  ; Clean up argument
    
    ; Restore segment registers
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore general-purpose registers
    popa
    
    ; Clean up error code and interrupt number from stack
    add esp, 8
    
    ; Enable interrupts and return
    sti
    iret

; Common ISR stub - saves all registers and calls C handler
isr_common_stub:
    ; Save all general-purpose registers
    pusha
    
    ; Save segment registers
    mov ax, ds
    push eax
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Pass stack pointer as argument (pointer to registers_t structure)
    push esp
    call isr_handler
    add esp, 4  ; Clean up argument
    
    ; Restore segment registers
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore general-purpose registers
    popa
    
    ; Clean up error code and interrupt number from stack
    add esp, 8
    
    ; Enable interrupts and return
    sti
    iret