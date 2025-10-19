// usermode.c - User Mode (Ring 3) Implementation
#include <usermode.h>
#include <process.h>
#include <printk.h>
#include <memory.h>

// User mode test code (position-independent assembly)
extern void user_mode_test_1_asm(void);
extern void user_mode_test_2_asm(void);
extern void user_mode_test_1_asm_end(void);
extern void user_mode_test_2_asm_end(void);

// User mode entry point (defined in assembly)
extern void enter_user_mode(uint32_t entry_point, uint32_t user_stack);

// Helper to read CS register and get CPL
uint32_t get_current_privilege_level(void) {
    uint16_t cs;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    return cs & 0x3;  // CPL is in bits 0-1
}

// Copy kernel function to user-accessible memory
static void* copy_to_user_memory(void* func_start, void* func_end, uint32_t user_base) {
    uint8_t* src = (uint8_t*)func_start;
    uint8_t* dst = (uint8_t*)user_base;
    uint32_t size = (uint32_t)func_end - (uint32_t)func_start;
    
    printk("    Copying %d bytes from 0x%x to 0x%x\n", size, (uint32_t)src, (uint32_t)dst);
    
    for (uint32_t i = 0; i < size; i++) {
        dst[i] = src[i];
    }
    
    return (void*)user_base;
}

// Set up a process to run in user mode (ring 3)
void process_setup_user_mode(process_t* process, void (*entry_point)(void)) {
    if (!process || !entry_point) {
        printk_error("Invalid process or entry point for user mode setup");
        return;
    }
    
    // Allocate user memory region (in user space, below kernel)
    // User space: 0x00000000 - 0xBFFFFFFF
    // Each process gets 1MB starting at 0x00400000 (4MB mark)
    uint32_t user_base = 0x00400000 + (process->pid * 0x100000);  // 1MB per process
    uint32_t user_code = user_base;                                // Code at base
    uint32_t user_stack_base = user_base + 0x80000;               // Stack at 512KB offset
    uint32_t user_stack_top = user_stack_base + 0x4000;           // 16KB user stack
    
    // Determine which assembly function to copy based on entry_point
    void* func_start;
    void* func_end;
    
    if (entry_point == (void*)user_mode_test_1) {
        func_start = (void*)user_mode_test_1_asm;
        func_end = (void*)user_mode_test_2_asm;  // End of test 1 is start of test 2
    } else if (entry_point == (void*)user_mode_test_2) {
        func_start = (void*)user_mode_test_2_asm;
        func_end = (void*)user_mode_test_2_asm_end;
    } else {
        printk_error("Unknown user mode entry point: 0x%x", (uint32_t)entry_point);
        return;
    }
    
    // Copy the function code to user-accessible memory
    void* user_entry = copy_to_user_memory(func_start, func_end, user_code);
    
    // Set up kernel stack for iret to user mode
    uint32_t* kstack = (uint32_t*)(process->kernel_stack + 4096);
    
    // Build stack frame for iret instruction
    // iret expects (from top of stack): EIP, CS, EFLAGS, ESP, SS
    *(--kstack) = 0x23;                      // SS (user data segment selector | RPL 3)
    *(--kstack) = user_stack_top;            // ESP (user stack pointer)
    *(--kstack) = 0x202;                     // EFLAGS (IF = 1, reserved bit 1 = 1)
    *(--kstack) = 0x1B;                      // CS (user code segment selector | RPL 3)
    *(--kstack) = (uint32_t)user_entry;      // EIP (entry point in USER memory)
    
    // Update process registers structure
    // ESP points to the iret frame
    process->registers.esp = (uint32_t)kstack;
    process->registers.eip = (uint32_t)user_entry;
    
    // Set general purpose registers to zero for clean start
    process->registers.eax = 0;
    process->registers.ebx = 0;
    process->registers.ecx = 0;
    process->registers.edx = 0;
    process->registers.esi = 0;
    process->registers.edi = 0;
    process->registers.ebp = 0;
    
    // Set user mode segment selectors
    process->registers.ds = 0x23;  // User data segment
    process->registers.es = 0x23;
    process->registers.fs = 0x23;
    process->registers.gs = 0x23;
    process->registers.ss = 0x23;  // User stack segment
    
    process->registers.eflags = 0x202;  // IF = 1
    
    // Mark process as user mode
    process->is_kernel = 0;
    
    printk("  Set up user mode for process %d (PID %d)\n", process->pid, process->pid);
    printk("    Entry point: 0x%x -> 0x%x (copied to user memory)\n", 
           (uint32_t)entry_point, (uint32_t)user_entry);
    printk("    User stack: 0x%x - 0x%x\n", user_stack_base, user_stack_top);
    printk("    Kernel stack: 0x%x\n", (uint32_t)process->kernel_stack);
}

// Test user mode function 1 (just a placeholder - real code is in assembly)
void user_mode_test_1(void) {
    // This function is never actually called
    // The assembly version (user_mode_test_1_asm) is what gets executed
    while (1) { asm volatile("hlt"); }
}

// Test user mode function 2 (just a placeholder - real code is in assembly)
void user_mode_test_2(void) {
    // This function is never actually called
    // The assembly version (user_mode_test_2_asm) is what gets executed
    while (1) { asm volatile("hlt"); }
}

// Initialize user mode subsystem
void usermode_init(void) {
    printk_info("Initializing user mode (ring 3) support");
    
    uint32_t cpl = get_current_privilege_level();
    printk("  Current privilege level: %d (ring %d)\n", cpl, cpl);
    
    if (cpl != 0) {
        printk_warn("Not running in kernel mode!");
    }
    
    printk("  User code segment: 0x1B (GDT entry 3 | RPL 3)\n");
    printk("  User data segment: 0x23 (GDT entry 4 | RPL 3)\n");
    printk("  [OK] User mode ready\n");
}
