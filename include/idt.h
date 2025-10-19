// idt.h - Interrupt Descriptor Table header
#ifndef AETHER_IDT_H
#define AETHER_IDT_H

#include <stdint.h>

// Initialize IDT with exception handlers
void idt_init(void);

// Set an IDT gate entry (for custom interrupts/syscalls)
void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags);

// Kernel panic function
void kernel_panic(const char* message);

#endif // AETHER_IDT_H