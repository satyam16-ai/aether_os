; gdt_flush.asm - Assembly routine to load GDT and flush segment registers
; Called from C code after GDT is set up in memory

SECTION .text
GLOBAL gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Get pointer to GDT descriptor from stack
    lgdt [eax]          ; Load GDT
    
    ; Reload segment registers with new GDT
    mov ax, 0x10        ; Kernel data segment selector (entry 2)
    mov ds, ax          ; Data segment
    mov es, ax          ; Extra segment  
    mov fs, ax          ; F segment
    mov gs, ax          ; G segment
    mov ss, ax          ; Stack segment
    
    ; Far jump to reload CS (code segment)
    ; This is required because CS can only be changed via far jump or interrupt return
    jmp 0x08:.flush    ; Jump to kernel code segment selector (entry 1)

.flush:
    ret                 ; Return to caller (now with new segments loaded)