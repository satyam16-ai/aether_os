// timer.c - Programmable Interval Timer (PIT) driver for system ticks
// Provides regular timer interrupts for scheduling and timekeeping

#include <stdint.h>
#include <printk.h>

// PIT I/O ports
#define PIT_CHANNEL0    0x40    // Channel 0 data port (IRQ 0)
#define PIT_CHANNEL1    0x41    // Channel 1 data port (RAM refresh, legacy)  
#define PIT_CHANNEL2    0x42    // Channel 2 data port (PC speaker)
#define PIT_COMMAND     0x43    // Command register

// PIT frequency: 1.193182 MHz (1193182 Hz)
#define PIT_FREQUENCY   1193182

// Command register bits
#define PIT_SELECT_CH0  (0 << 6)   // Select channel 0
#define PIT_SELECT_CH1  (1 << 6)   // Select channel 1  
#define PIT_SELECT_CH2  (2 << 6)   // Select channel 2
#define PIT_ACCESS_LOW  (1 << 4)   // Access low byte only
#define PIT_ACCESS_HIGH (2 << 4)   // Access high byte only
#define PIT_ACCESS_BOTH (3 << 4)   // Access low then high byte
#define PIT_MODE0       (0 << 1)   // Mode 0: Interrupt on terminal count
#define PIT_MODE1       (1 << 1)   // Mode 1: Hardware retriggerable one-shot
#define PIT_MODE2       (2 << 1)   // Mode 2: Rate generator
#define PIT_MODE3       (3 << 1)   // Mode 3: Square wave generator
#define PIT_BINARY      (0 << 0)   // Binary mode
#define PIT_BCD         (1 << 0)   // BCD mode

// Global tick counter (32-bit to avoid runtime dependencies)
static volatile uint32_t system_ticks = 0;
static uint32_t timer_frequency_hz = 0;

// External functions from PIC
extern void pic_enable_irq(uint8_t irq);
extern void pic_send_eoi(uint8_t irq);

// Port I/O functions
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void timer_init(uint32_t frequency_hz) {
    printk_info("Initializing Programmable Interval Timer (PIT)");
    
    // Calculate divisor for desired frequency
    uint32_t divisor = PIT_FREQUENCY / frequency_hz;
    
    // Clamp divisor to valid range
    if (divisor < 1) divisor = 1;
    if (divisor > 65535) divisor = 65535;
    
    // Calculate actual frequency we'll get
    timer_frequency_hz = PIT_FREQUENCY / divisor;
    
    printk("  Requested: %u Hz, Divisor: %u, Actual: %u Hz\n", 
           frequency_hz, divisor, timer_frequency_hz);
    
    // Configure PIT Channel 0 for periodic interrupts
    // Mode 2 (rate generator) with binary counting, access low/high byte
    uint8_t command = PIT_SELECT_CH0 | PIT_ACCESS_BOTH | PIT_MODE2 | PIT_BINARY;
    outb(PIT_COMMAND, command);
    
    // Send divisor (low byte first, then high byte)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    // Enable IRQ 0 (timer interrupt)
    pic_enable_irq(0);
    
    printk("  [OK] Timer configured: %u Hz (%u.%03u ms period)\n", 
           timer_frequency_hz,
           1000 / timer_frequency_hz,
           (1000000 / timer_frequency_hz) % 1000);
}

// Timer interrupt handler (called from IRQ 0 handler)
void timer_handler(void) {
    system_ticks++;
    
    // Send End-of-Interrupt to PIC
    pic_send_eoi(0);
    
    // Print tick counter every second (adjust based on frequency)
    if (system_ticks % timer_frequency_hz == 0) {
        uint32_t seconds = system_ticks / timer_frequency_hz;
        printk("[TICK] Uptime: %u seconds (%u ticks)\n", seconds, system_ticks);
    }
}

// Get current tick count
uint32_t timer_get_ticks(void) {
    return system_ticks;
}

// Get uptime in seconds
uint32_t timer_get_uptime_seconds(void) {
    return system_ticks / timer_frequency_hz;
}

// Get timer frequency
uint32_t timer_get_frequency(void) {
    return timer_frequency_hz;
}

// Sleep for specified number of ticks
void timer_sleep_ticks(uint32_t ticks) {
    uint32_t target = system_ticks + ticks;
    while (system_ticks < target) {
        __asm__ volatile("hlt");  // Halt until next interrupt
    }
}

// Sleep for specified milliseconds (approximate)
void timer_sleep_ms(uint32_t milliseconds) {
    uint32_t ticks = (milliseconds * timer_frequency_hz) / 1000;
    if (ticks == 0) ticks = 1;  // Sleep at least one tick
    timer_sleep_ticks(ticks);
}