; context_switch.asm - Low-level context switching for process multitasking
; This code saves all CPU registers to the old process's context,
; then restores all registers from the new process's context.

[BITS 32]

global context_switch
global read_esp
global read_ebp
global read_eip

; void context_switch(registers_t* old_regs, registers_t* new_regs)
; Save current context to old_regs, restore context from new_regs
; 
; registers_t layout (80 bytes):
;   +0:  eax
;   +4:  ebx
;   +8:  ecx
;   +12: edx
;   +16: esi
;   +20: edi
;   +24: ebp
;   +28: esp
;   +32: ds
;   +36: es
;   +40: fs
;   +44: gs
;   +48: ss
;   +52: eip
;   +56: eflags
;   +60: cr3

context_switch:
    ; Arguments on stack:
    ;   [esp+4] = old_regs (pointer to save current context)
    ;   [esp+8] = new_regs (pointer to load new context)
    
    ; Get pointers
    mov eax, [esp+4]        ; old_regs
    mov edx, [esp+8]        ; new_regs
    
    ; Check if old_regs is NULL (first context switch)
    test eax, eax
    jz .load_new_context
    
    ; === SAVE CURRENT CONTEXT ===
    
    ; Save general purpose registers
    mov [eax+0], eax        ; Save EAX (will be corrected below)
    mov [eax+4], ebx        ; EBX
    mov [eax+8], ecx        ; ECX
    mov [eax+12], edx       ; EDX (will be corrected below)
    mov [eax+16], esi       ; ESI
    mov [eax+20], edi       ; EDI
    mov [eax+24], ebp       ; EBP
    
    ; Save ESP (current stack pointer + return address)
    mov ecx, esp
    add ecx, 4              ; Adjust for return address
    mov [eax+28], ecx       ; ESP
    
    ; Save segment registers
    mov cx, ds
    mov [eax+32], ecx       ; DS
    mov cx, es
    mov [eax+36], ecx       ; ES
    mov cx, fs
    mov [eax+40], ecx       ; FS
    mov cx, gs
    mov [eax+44], ecx       ; GS
    mov cx, ss
    mov [eax+48], ecx       ; SS
    
    ; Save return address as EIP
    mov ecx, [esp]          ; Get return address
    mov [eax+52], ecx       ; EIP
    
    ; Save EFLAGS
    pushfd
    pop ecx
    mov [eax+56], ecx       ; EFLAGS
    
    ; Save CR3 (page directory)
    mov ecx, cr3
    mov [eax+60], ecx       ; CR3
    
    ; Correct saved EAX and EDX (they were used for addressing)
    mov ecx, [esp+4]        ; Reload old_regs pointer
    mov edi, eax            ; Save original EAX value
    mov [ecx+0], edi        ; Store correct EAX
    mov edi, [esp+8]        ; Reload new_regs pointer  
    mov [ecx+12], edi       ; Store correct EDX (was pointing to new_regs)
    
.load_new_context:
    ; === RESTORE NEW CONTEXT ===
    
    ; EDX should point to new_regs
    test edx, edx
    jz .done                ; Safety check
    
    ; Restore CR3 (page directory) - switch address space
    mov eax, [edx+60]       ; CR3
    mov cr3, eax
    
    ; Check if this is a user mode process FIRST
    mov ax, [edx+32]        ; Get DS (for user mode, will be 0x23)
    cmp ax, 0x23            ; Is this user mode? (0x23 = user data segment)
    je .user_mode_return
    
    ; ===== KERNEL MODE RETURN =====
    ; Restore segment registers for kernel mode
    mov ax, [edx+32]        ; DS
    mov ds, ax
    mov ax, [edx+36]        ; ES
    mov es, ax
    mov ax, [edx+40]        ; FS
    mov fs, ax
    mov ax, [edx+44]        ; GS
    mov gs, ax
    mov ax, [edx+48]        ; SS
    mov ss, ax
    
    ; Restore EFLAGS
    mov eax, [edx+56]       ; EFLAGS
    push eax
    popfd
    
    ; Set up stack to return to new EIP
    mov esp, [edx+28]       ; ESP
    mov eax, [edx+52]       ; EIP
    push eax                ; Push EIP as return address
    
    ; Restore general purpose registers
    mov eax, [edx+0]        ; EAX
    mov ebx, [edx+4]        ; EBX
    mov ecx, [edx+8]        ; ECX
    mov esi, [edx+16]       ; ESI
    mov edi, [edx+20]       ; EDI
    mov ebp, [edx+24]       ; EBP
    mov edx, [edx+12]       ; EDX (restore last)
    
    ret                     ; Return to kernel mode
    
.user_mode_return:
    ; ===== USER MODE RETURN =====
    ; For user mode, we DON'T load segment registers manually
    ; The iret instruction will load CS and SS from the stack
    ; But we need to load DS, ES, FS, GS to kernel segments temporarily
    ; because we're still in kernel mode until iret executes
    
    mov ax, 0x10            ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Load the kernel stack with the iret frame
    mov esp, [edx+28]       ; ESP points to: [EIP] [CS] [EFLAGS] [ESP] [SS]
    
    ; Restore general purpose registers BEFORE iret
    mov eax, [edx+0]        ; EAX
    mov ebx, [edx+4]        ; EBX
    mov ecx, [edx+8]        ; ECX
    mov esi, [edx+16]       ; ESI
    mov edi, [edx+20]       ; EDI
    mov ebp, [edx+24]       ; EBP
    mov edx, [edx+12]       ; EDX (restore last)
    
    ; Now iret will:
    ; 1. Pop EIP
    ; 2. Pop CS (0x1B - user code with RPL 3)
    ; 3. Pop EFLAGS
    ; 4. Pop ESP (user stack)
    ; 5. Pop SS (0x23 - user data with RPL 3)
    ; 6. Switch to ring 3 and jump to EIP
    iret                    ; Return to user mode
    
.done:
    ret                     ; Jump to new process's EIP

; uint32_t read_esp(void)
; Returns current stack pointer value
read_esp:
    mov eax, esp
    ret

; uint32_t read_ebp(void)
; Returns current base pointer value
read_ebp:
    mov eax, ebp
    ret

; uint32_t read_eip(void)
; Returns current instruction pointer (return address)
read_eip:
    pop eax                 ; Get return address
    jmp eax                 ; Jump back (EAX contains EIP)
