; kernel_entry.asm - Simple entry point for 32-bit kernel
; Let's start with 32-bit and get it working first

BITS 32
SECTION .text
GLOBAL _start
EXTERN kmain

_start:
    ; Set up a simple stack
    mov esp, stack_top

    ; Clear BSS
    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Call C kernel main
    call kmain

.hang:
    cli
    hlt
    jmp .hang

SECTION .bss
    align 16
stack_bottom:
    resb 4096 ; 4 KiB stack
stack_top:
