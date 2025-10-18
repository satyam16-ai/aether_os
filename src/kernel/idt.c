// idt.c - Interrupt Descriptor Table setup for Aether OS
// Sets up IDT with CPU exception handlers and prepares for IRQ handling

#include <stdint.h>
#include <printk.h>

// IDT Entry structure
struct idt_entry {
    uint16_t offset_low;   // Lower 16 bits of handler address
    uint16_t selector;     // Kernel code segment selector
    uint8_t  zero;         // Always 0
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_high;  // Upper 16 bits of handler address
} __attribute__((packed));

// IDT Pointer structure for LIDT instruction
struct idt_ptr {
    uint16_t limit;        // Size of IDT - 1
    uint32_t base;         // Address of IDT
} __attribute__((packed));

// IDT with 256 entries (32 exceptions + 224 IRQs/software interrupts)
#define IDT_ENTRIES 256
static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idt_pointer;

// IDT Entry type flags
#define IDT_PRESENT     (1 << 7)
#define IDT_RING0       (0 << 5)
#define IDT_RING3       (3 << 5)
#define IDT_INT_GATE    0x0E    // 32-bit interrupt gate
#define IDT_TRAP_GATE   0x0F    // 32-bit trap gate

// Exception handler declarations (implemented in idt_handlers.asm)
extern void isr0(void);   // Division by Zero
extern void isr1(void);   // Debug
extern void isr2(void);   // Non-Maskable Interrupt
extern void isr3(void);   // Breakpoint
extern void isr4(void);   // Overflow
extern void isr5(void);   // Bound Range Exceeded
extern void isr6(void);   // Invalid Opcode
extern void isr7(void);   // Device Not Available
extern void isr8(void);   // Double Fault
extern void isr9(void);   // Coprocessor Segment Overrun (legacy)
extern void isr10(void);  // Invalid TSS
extern void isr11(void);  // Segment Not Present
extern void isr12(void);  // Stack-Segment Fault
extern void isr13(void);  // General Protection Fault
extern void isr14(void);  // Page Fault
extern void isr15(void);  // Reserved
extern void isr16(void);  // x87 Floating-Point Exception
extern void isr17(void);  // Alignment Check
extern void isr18(void);  // Machine Check
extern void isr19(void);  // SIMD Floating-Point Exception
extern void isr20(void);  // Virtualization Exception
extern void isr21(void);  // Reserved
extern void isr22(void);  // Reserved
extern void isr23(void);  // Reserved
extern void isr24(void);  // Reserved
extern void isr25(void);  // Reserved
extern void isr26(void);  // Reserved
extern void isr27(void);  // Reserved
extern void isr28(void);  // Reserved
extern void isr29(void);  // Reserved
extern void isr30(void);  // Security Exception
extern void isr31(void);  // Reserved

// IRQ handler declarations (implemented in idt_handlers.asm)
extern void irq0(void);   // Timer
extern void irq1(void);   // Keyboard
extern void irq2(void);   // Cascade
extern void irq3(void);   // COM2
extern void irq4(void);   // COM1
extern void irq5(void);   // LPT2
extern void irq6(void);   // Floppy
extern void irq7(void);   // LPT1
extern void irq8(void);   // CMOS RTC
extern void irq9(void);   // Free
extern void irq10(void);  // Free
extern void irq11(void);  // Free
extern void irq12(void);  // PS2 Mouse
extern void irq13(void);  // FPU
extern void irq14(void);  // Primary ATA
extern void irq15(void);  // Secondary ATA

// External function to load IDT
extern void idt_flush(uint32_t idt_ptr_addr);

// Helper function to set an IDT entry
static void idt_set_gate(uint8_t num, uint32_t handler, uint16_t selector, uint8_t flags) {
    idt[num].offset_low  = handler & 0xFFFF;
    idt[num].offset_high = (handler >> 16) & 0xFFFF;
    idt[num].selector    = selector;
    idt[num].zero        = 0;
    idt[num].type_attr   = flags;
}

void idt_init(void) {
    printk_info("Initializing Interrupt Descriptor Table (IDT)");
    
    // Set up IDT pointer
    idt_pointer.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idt_pointer.base = (uint32_t)&idt;
    
    printk("  IDT Base: 0x%X, Limit: %u bytes (%d entries)\n", 
           idt_pointer.base, idt_pointer.limit + 1, IDT_ENTRIES);
    
    // Clear IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Set up CPU exception handlers (0-31)
    // Use kernel code segment (0x08) and interrupt gates
    extern uint16_t gdt_get_kernel_cs(void);
    uint16_t kernel_cs = gdt_get_kernel_cs();
    
    idt_set_gate(0,  (uint32_t)isr0,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(1,  (uint32_t)isr1,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(2,  (uint32_t)isr2,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(3,  (uint32_t)isr3,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(4,  (uint32_t)isr4,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(5,  (uint32_t)isr5,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(6,  (uint32_t)isr6,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(7,  (uint32_t)isr7,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(8,  (uint32_t)isr8,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(9,  (uint32_t)isr9,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(10, (uint32_t)isr10, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(11, (uint32_t)isr11, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(12, (uint32_t)isr12, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(13, (uint32_t)isr13, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(14, (uint32_t)isr14, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(15, (uint32_t)isr15, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(16, (uint32_t)isr16, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(17, (uint32_t)isr17, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(18, (uint32_t)isr18, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(19, (uint32_t)isr19, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(20, (uint32_t)isr20, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(21, (uint32_t)isr21, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(22, (uint32_t)isr22, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(23, (uint32_t)isr23, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(24, (uint32_t)isr24, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(25, (uint32_t)isr25, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(26, (uint32_t)isr26, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(27, (uint32_t)isr27, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(28, (uint32_t)isr28, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(29, (uint32_t)isr29, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(30, (uint32_t)isr30, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(31, (uint32_t)isr31, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    
    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint32_t)irq0,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(33, (uint32_t)irq1,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(34, (uint32_t)irq2,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(35, (uint32_t)irq3,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(36, (uint32_t)irq4,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(37, (uint32_t)irq5,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(38, (uint32_t)irq6,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(39, (uint32_t)irq7,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(40, (uint32_t)irq8,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(41, (uint32_t)irq9,  kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(42, (uint32_t)irq10, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(43, (uint32_t)irq11, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(44, (uint32_t)irq12, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(45, (uint32_t)irq13, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(46, (uint32_t)irq14, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    idt_set_gate(47, (uint32_t)irq15, kernel_cs, IDT_PRESENT | IDT_RING0 | IDT_INT_GATE);
    
    // Load IDT
    idt_flush((uint32_t)&idt_pointer);
    
    printk("  [OK] IDT loaded with exception handlers (0-31) and IRQ handlers (32-47)\n");
}