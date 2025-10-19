// tss.c - Task State Segment Implementation
#include <tss.h>
#include <gdt.h>
#include <printk.h>
#include <memory.h>

// Global TSS
tss_entry_t kernel_tss;

// External GDT functions
extern void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Initialize TSS
void tss_init(uint32_t kernel_stack) {
    printk_info("Initializing Task State Segment (TSS)");
    
    // Clear TSS
    memset(&kernel_tss, 0, sizeof(tss_entry_t));
    
    // Set up kernel stack
    kernel_tss.ss0 = 0x10;  // Kernel data segment
    kernel_tss.esp0 = kernel_stack;
    
    // Set up segment registers (kernel mode)
    kernel_tss.cs = 0x0b;   // Kernel code segment | 0x3 (ring 0)
    kernel_tss.ss = 0x10;   // Kernel data segment
    kernel_tss.ds = 0x10;
    kernel_tss.es = 0x10;
    kernel_tss.fs = 0x10;
    kernel_tss.gs = 0x10;
    
    // No I/O map
    kernel_tss.iomap_base = sizeof(tss_entry_t);
    
    // Add TSS descriptor to GDT (entry 5, after NULL, CODE, DATA, USER_CODE, USER_DATA)
    uint32_t base = (uint32_t)&kernel_tss;
    uint32_t limit = sizeof(tss_entry_t) - 1;
    
    // TSS descriptor: Present, Ring 0, 32-bit TSS
    // Access: 0xE9 = 1110 1001
    //   Present=1, DPL=00, Type=1001 (Available 32-bit TSS)
    // Granularity: 0x00 = byte granularity
    gdt_set_gate(5, base, limit, 0xE9, 0x00);
    
    printk("  TSS at 0x%08X, size %d bytes\n", base, limit + 1);
    printk("  Kernel stack: SS=0x%04X, ESP=0x%08X\n", kernel_tss.ss0, kernel_tss.esp0);
    
    // Load TSS
    tss_flush();
    
    printk("  [OK] TSS initialized and loaded\n");
}

// Update kernel stack in TSS (called on context switch)
void tss_set_kernel_stack(uint32_t stack) {
    kernel_tss.esp0 = stack;
}

// Assembly function to load TSS
extern void tss_flush_asm(void);

void tss_flush(void) {
    tss_flush_asm();
}
