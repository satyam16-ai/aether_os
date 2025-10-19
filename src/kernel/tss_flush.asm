; tss_flush.asm - Load Task State Segment
[BITS 32]

global tss_flush_asm

; void tss_flush_asm(void)
; Loads the TSS descriptor into the Task Register
tss_flush_asm:
    ; TSS is at GDT entry 5
    ; Selector = 5 * 8 = 0x28 (5th entry, 8 bytes per entry)
    ; Ring 0, so RPL = 0
    mov ax, 0x28
    ltr ax          ; Load Task Register
    ret
