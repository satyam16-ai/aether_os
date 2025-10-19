; user_mode_enter.asm - Enter User Mode (Ring 3)
; This code switches from kernel mode (ring 0) to user mode (ring 3)

global enter_user_mode
global return_to_user_mode

; Enter user mode for the first time
; void enter_user_mode(uint32_t entry_point, uint32_t user_stack)
enter_user_mode:
    cli                     ; Disable interrupts during setup
    
    mov eax, [esp + 4]      ; Get entry_point (first argument)
    mov ebx, [esp + 8]      ; Get user_stack (second argument)
    
    ; Set up data segments for user mode
    mov ax, 0x23            ; User data segment selector (GDT entry 4 | RPL 3)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Build iret stack frame
    ; iret pops: EIP, CS, EFLAGS, ESP, SS
    mov eax, [esp + 8]      ; user_stack
    push 0x23               ; SS (user data segment)
    push eax                ; ESP (user stack)
    
    pushf                   ; Push current EFLAGS
    pop eax
    or eax, 0x200           ; Set IF (enable interrupts in user mode)
    push eax                ; Push modified EFLAGS
    
    push 0x1B               ; CS (user code segment selector | RPL 3)
    
    mov eax, [esp + 20]     ; Get entry_point (adjusted for pushes)
    push eax                ; EIP
    
    ; Switch to user mode
    iret                    ; Pop CS, EIP, EFLAGS, SS, ESP

; Return to user mode after handling interrupt/syscall
; void return_to_user_mode(registers_t* regs)
return_to_user_mode:
    mov ebp, [esp + 4]      ; Get pointer to registers
    
    ; Restore segment registers
    mov ax, 0x23            ; User data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Restore general purpose registers from struct
    mov eax, [ebp + 0]      ; EAX
    mov ebx, [ebp + 4]      ; EBX
    mov ecx, [ebp + 8]      ; ECX
    mov edx, [ebp + 12]     ; EDX
    mov esi, [ebp + 16]     ; ESI
    mov edi, [ebp + 20]     ; EDI
    
    ; Build iret frame
    push dword [ebp + 40]   ; SS
    push dword [ebp + 32]   ; ESP
    push dword [ebp + 36]   ; EFLAGS
    push dword [ebp + 44]   ; CS
    push dword [ebp + 28]   ; EIP
    
    ; Restore EBP last
    mov ebp, [ebp + 24]
    
    ; Return to user mode
    iret

; Helper: Get current CPL (Current Privilege Level)
global get_cpl
get_cpl:
    mov ax, cs              ; Read CS register
    and ax, 3               ; Mask to get CPL (bits 0-1)
    movzx eax, ax           ; Zero-extend to 32-bit
    ret
