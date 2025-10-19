; user_test_code.asm - Position-independent user mode test code
; This code is copied to user memory and executed in ring 3

[BITS 32]

section .text

; User mode test function 1
; This is position-independent code that uses only registers and stack
global user_mode_test_1_asm
user_mode_test_1_asm:
    ; Initialize counter
    xor edi, edi                    ; counter = 0
    
    ; Build the message on the stack (backwards)
    ; "User process 1 running\n" = 23 bytes
    push 0x0A6E7572                 ; "\nr" + "un"
    push 0x20676E69                 ; "ing "
    push 0x6E6E7572                 ; "runn"
    push 0x20317373                 ; "ss1 "
    push 0x65636F72                 ; "roce"
    push 0x70207265                 ; "er p"
    push 0x73552020                 ; "  Us"
    
    ; Get the string address (stack pointer)
    lea ecx, [esp + 2]              ; Skip first 2 bytes to align
    mov edx, 23                     ; length
    
    ; syscall write(1, msg, 23)
    mov eax, 2                      ; syscall number for write
    mov ebx, 1                      ; fd = stdout
    int 0x80
    
    ; Clean up stack
    add esp, 28
    
.loop:
    inc edi                         ; counter++
    
    ; Check if counter % 1000000 == 0
    mov eax, edi
    xor edx, edx
    mov ecx, 1000000
    div ecx
    test edx, edx
    jnz .no_yield
    
    ; syscall yield()
    mov eax, 4                      ; syscall number for yield
    int 0x80
    
.no_yield:
    ; Check if counter > 50000000
    cmp edi, 50000000
    jl .loop
    
    ; syscall exit(0)
    mov eax, 1                      ; syscall number for exit
    mov ebx, 0                      ; status = 0
    int 0x80
    
    ; Should never reach here
.halt:
    hlt
    jmp .halt

; User mode test function 2
global user_mode_test_2_asm
user_mode_test_2_asm:
    xor edi, edi
    
    ; "User process 2 running\n"
    push 0x0A6E7572                 ; "\nr" + "un"
    push 0x20676E69                 ; "ing "
    push 0x6E6E7572                 ; "runn"
    push 0x20327373                 ; "ss2 "  (NOTE: "2" instead of "1")
    push 0x65636F72                 ; "roce"
    push 0x70207265                 ; "er p"
    push 0x73552020                 ; "  Us"
    
    lea ecx, [esp + 2]
    mov edx, 23
    
    mov eax, 2
    mov ebx, 1
    int 0x80
    
    add esp, 28
    
.loop:
    inc edi
    
    mov eax, edi
    xor edx, edx
    mov ecx, 1000000
    div ecx
    test edx, edx
    jnz .no_yield
    
    mov eax, 4
    int 0x80
    
.no_yield:
    cmp edi, 50000000
    jl .loop
    
    mov eax, 1
    mov ebx, 0
    int 0x80
    
.halt:
    hlt
    jmp .halt

; Export end markers so we can calculate size
global user_mode_test_1_asm_end
user_mode_test_1_asm_end:

global user_mode_test_2_asm_end
user_mode_test_2_asm_end:
