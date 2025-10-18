// pic.c - Programmable Interrupt Controller (8259A) setup
// Remaps IRQs from default locations to avoid conflicts with CPU exceptions

#include <stdint.h>
#include <printk.h>

// PIC I/O port addresses
#define PIC1_COMMAND    0x20    // Master PIC command port
#define PIC1_DATA       0x21    // Master PIC data port
#define PIC2_COMMAND    0xA0    // Slave PIC command port  
#define PIC2_DATA       0xA1    // Slave PIC data port

// PIC initialization command words
#define ICW1_ICW4       0x01    // ICW4 (not) needed
#define ICW1_SINGLE     0x02    // Single (cascade) mode
#define ICW1_INTERVAL4  0x04    // Call address interval 4 (8)
#define ICW1_LEVEL      0x08    // Level triggered (edge) mode
#define ICW1_INIT       0x10    // Initialization - required!

#define ICW4_8086       0x01    // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       0x02    // Auto (normal) EOI
#define ICW4_BUF_SLAVE  0x08    // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C    // Buffered mode/master
#define ICW4_SFNM       0x10    // Special fully nested (not)

// Default IRQ mappings (before remapping)
// IRQ 0-7  -> INT 0x08-0x0F (conflicts with CPU exceptions!)
// IRQ 8-15 -> INT 0x70-0x77

// New IRQ mappings (after remapping)
#define IRQ_BASE_MASTER 0x20    // IRQ 0-7  -> INT 0x20-0x27
#define IRQ_BASE_SLAVE  0x28    // IRQ 8-15 -> INT 0x28-0x2F

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Small delay for old hardware compatibility
static void io_wait(void) {
    // Port 0x80 is used for 'checkpoints' during POST
    // Writing to it causes a small delay
    outb(0x80, 0);
}

void pic_init(void) {
    printk_info("Initializing Programmable Interrupt Controller (PIC)");
    
    // Save current interrupt masks
    uint8_t master_mask = inb(PIC1_DATA);
    uint8_t slave_mask = inb(PIC2_DATA);
    
    printk("  Current masks: Master=0x%02X, Slave=0x%02X\n", master_mask, slave_mask);
    
    // Start initialization sequence (ICW1)
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    
    // ICW2: Set interrupt vector offsets
    outb(PIC1_DATA, IRQ_BASE_MASTER);   // Master PIC vector offset
    io_wait();
    outb(PIC2_DATA, IRQ_BASE_SLAVE);    // Slave PIC vector offset
    io_wait();
    
    // ICW3: Set up cascade (master has slave on IRQ2, slave cascade identity = 2)
    outb(PIC1_DATA, 4);                 // Tell master PIC that slave is at IRQ2 (0000 0100)
    io_wait();
    outb(PIC2_DATA, 2);                 // Tell slave PIC its cascade identity (0000 0010)
    io_wait();
    
    // ICW4: Set mode
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();
    
    // Mask all interrupts initially (except cascade)
    outb(PIC1_DATA, 0xFB);  // Mask all except IRQ2 (cascade to slave)
    outb(PIC2_DATA, 0xFF);  // Mask all slave interrupts
    
    printk("  [OK] PIC remapped: IRQ 0-7 -> INT 0x%02X-0x%02X\n", 
           IRQ_BASE_MASTER, IRQ_BASE_MASTER + 7);
    printk("                    IRQ 8-15 -> INT 0x%02X-0x%02X\n", 
           IRQ_BASE_SLAVE, IRQ_BASE_SLAVE + 7);
    printk("       All IRQs masked pending specific driver setup\n");
}

// Send End-of-Interrupt signal
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        // Send EOI to slave PIC
        outb(PIC2_COMMAND, 0x20);
    }
    // Always send EOI to master PIC
    outb(PIC1_COMMAND, 0x20);
}

// Enable specific IRQ
void pic_enable_irq(uint8_t irq) {
    uint16_t port;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    uint8_t value = inb(port) & ~(1 << irq);
    outb(port, value);
}

// Disable specific IRQ
void pic_disable_irq(uint8_t irq) {
    uint16_t port;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    uint8_t value = inb(port) | (1 << irq);
    outb(port, value);
}

// Get IRQ mask status
uint16_t pic_get_mask(void) {
    return (inb(PIC2_DATA) << 8) | inb(PIC1_DATA);
}