// gdt.h - Global Descriptor Table header
#ifndef AETHER_GDT_H
#define AETHER_GDT_H

#include <stdint.h>

// Initialize GDT with flat memory model
void gdt_init(void);

// Get segment selectors
uint16_t gdt_get_kernel_cs(void);
uint16_t gdt_get_kernel_ds(void);

#endif // AETHER_GDT_H