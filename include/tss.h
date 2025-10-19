// tss.h - Task State Segment for x86 Hardware Task Switching
#ifndef TSS_H
#define TSS_H

#include <stdint.h>

// Task State Segment structure (32-bit x86)
typedef struct {
    uint32_t prev_tss;   // Previous TSS (unused in software task switching)
    uint32_t esp0;       // Kernel stack pointer
    uint32_t ss0;        // Kernel stack segment
    uint32_t esp1;       // Unused
    uint32_t ss1;        // Unused
    uint32_t esp2;       // Unused
    uint32_t ss2;        // Unused
    uint32_t cr3;        // Page directory base
    uint32_t eip;        // Instruction pointer
    uint32_t eflags;     // CPU flags
    uint32_t eax;        // General purpose registers
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;        // Stack pointer
    uint32_t ebp;        // Base pointer
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         // Segment registers
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;        // LDT selector (unused)
    uint16_t trap;       // Trap on task switch
    uint16_t iomap_base; // I/O map base address
} __attribute__((packed)) tss_entry_t;

// Global TSS entry
extern tss_entry_t kernel_tss;

// TSS management functions
void tss_init(uint32_t kernel_stack);
void tss_set_kernel_stack(uint32_t stack);
void tss_flush(void);

#endif // TSS_H
