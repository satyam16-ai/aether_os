// syscall.c - System Call Interface
#include <syscall.h>
#include <process.h>
#include <scheduler.h>
#include <printk.h>
#include <idt.h>

// System call handler (called from assembly wrapper)
void syscall_handler(registers_t* regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;
    
    // Debug: print syscall number
    printk("[SYSCALL] num=%d (EAX=0x%x) arg1=0x%x arg2=0x%x arg3=0x%x\n",
           syscall_num, regs->eax, arg1, arg2, arg3);
    
    uint32_t result = 0;
    
    switch (syscall_num) {
        case SYSCALL_EXIT:
            result = sys_exit((int)arg1);
            break;
            
        case SYSCALL_WRITE:
            result = sys_write((int)arg1, (const char*)arg2, arg3);
            break;
            
        case SYSCALL_READ:
            result = sys_read((int)arg1, (char*)arg2, arg3);
            break;
            
        case SYSCALL_YIELD:
            result = sys_yield();
            break;
            
        default:
            printk_warn("Unknown syscall: %d", syscall_num);
            result = -1;
            break;
    }
    
    // Return value goes in EAX
    regs->eax = result;
}

// Syscall implementations

// Exit the current process
int sys_exit(int status) {
    if (!current_process) {
        return -1;
    }
    
    printk("[SYSCALL] Process %d (%s) exiting with status %d\n",
           current_process->pid, current_process->name, status);
    
    process_exit(status);
    
    // Never returns
    return 0;
}

// Write to file descriptor (for now, just console)
int sys_write(int fd, const char* buf, uint32_t len) {
    if (!buf || len == 0) {
        return 0;
    }
    
    // Only support stdout (fd=1) and stderr (fd=2) for now
    if (fd != 1 && fd != 2) {
        return -1;
    }
    
    // Write to console
    for (uint32_t i = 0; i < len; i++) {
        if (buf[i] == '\0') break;
        printk("%c", buf[i]);
    }
    
    return len;
}

// Read from file descriptor (stub for now)
int sys_read(int fd, char* buf, uint32_t len) {
    (void)fd;
    (void)buf;
    (void)len;
    
    // Not implemented yet
    return -1;
}

// Yield CPU to another process
int sys_yield(void) {
    if (scheduler_is_enabled()) {
        scheduler_yield();
    }
    return 0;
}

// Initialize system call interface
void syscall_init(void) {
    printk_info("Initializing system call interface");
    
    // Register INT 0x80 for system calls
    // DPL=3 means user mode can call this interrupt
    extern void syscall_wrapper(void);
    idt_set_gate(0x80, (uint32_t)syscall_wrapper, 0x08, 0xEE);
    // 0xEE = Present (1) + DPL 3 (11) + Type 32-bit Interrupt Gate (01110)
    
    printk("  Syscall interrupt: INT 0x80\n");
    printk("  Available syscalls:\n");
    printk("    1 - exit(status)\n");
    printk("    2 - write(fd, buf, len)\n");
    printk("    3 - read(fd, buf, len)\n");
    printk("    4 - yield()\n");
    printk("  [OK] System calls ready\n");
}
