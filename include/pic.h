// pic.h - Programmable Interrupt Controller header
#ifndef AETHER_PIC_H
#define AETHER_PIC_H

#include <stdint.h>

// Initialize PIC with remapped IRQs
void pic_init(void);

// Send End-of-Interrupt signal
void pic_send_eoi(uint8_t irq);

// Enable/disable specific IRQs
void pic_enable_irq(uint8_t irq);
void pic_disable_irq(uint8_t irq);

// Get current IRQ mask
uint16_t pic_get_mask(void);

#endif // AETHER_PIC_H