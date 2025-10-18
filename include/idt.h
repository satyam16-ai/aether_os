// idt.h - Interrupt Descriptor Table header
#ifndef AETHER_IDT_H
#define AETHER_IDT_H

#include <stdint.h>

// Initialize IDT with exception handlers
void idt_init(void);

// Kernel panic function
void kernel_panic(const char* message);

#endif // AETHER_IDT_H