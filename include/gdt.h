// gdt.h - Global Descriptor Table header
#ifndef AETHER_GDT_H
#define AETHER_GDT_H

#include <stdint.h>

// Initialize GDT with flat memory model
void gdt_init(void);

// Set a GDT gate (for TSS)
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// Get segment selectors
uint16_t gdt_get_kernel_cs(void);
uint16_t gdt_get_kernel_ds(void);

#endif // AETHER_GDT_H