// panic.c - Kernel panic handler with register dumps and system halt
// Also contains IRQ dispatcher for hardware interrupts
#include <stdint.h>
#include <printk.h>
#include <pic.h>
#include <keyboard.h>



// External timer handler
extern void timer_handler(void);

// Structure to hold CPU register state during interrupt
typedef struct {
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
    uint32_t int_no, err_code;                       // Interrupt number and error code
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by CPU during interrupt
} __attribute__((packed)) registers_t;

// Exception names for better error reporting
static const char* exception_messages[] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun (legacy)",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

// Page fault error code flags
#define PAGE_FAULT_PRESENT     (1 << 0)  // Page was present
#define PAGE_FAULT_WRITE       (1 << 1)  // Write operation caused fault
#define PAGE_FAULT_USER        (1 << 2)  // Fault occurred in user mode
#define PAGE_FAULT_RESERVED    (1 << 3)  // Reserved bit was set
#define PAGE_FAULT_FETCH       (1 << 4)  // Instruction fetch caused fault

// Get CR2 register (page fault linear address)
static uint32_t get_cr2(void) {
    uint32_t cr2;
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    return cr2;
}

// Halt the system completely
static void halt_system(void) {
    __asm__ volatile("cli; hlt");
    for (;;) {
        __asm__ volatile("pause");
    }
}

// Detailed page fault handler
static void handle_page_fault(registers_t* regs) {
    uint32_t faulting_address = get_cr2();
    uint32_t error_code = regs->err_code;
    printk("Page Fault Details:\n");
    printk("  Faulting Address: 0x%08X\n", faulting_address);
    printk("  Error Code: 0x%X (", error_code);
    if (error_code & PAGE_FAULT_PRESENT) {
        printk("protection violation");
    } else {
        printk("page not present");
    }
    if (error_code & PAGE_FAULT_WRITE) {
        printk(", write");
    } else {
        printk(", read");
    }
    if (error_code & PAGE_FAULT_USER) {
        printk(", user mode");
    } else {
        printk(", kernel mode");
    }
    if (error_code & PAGE_FAULT_RESERVED) {
        printk(", reserved bits set");
    }
    if (error_code & PAGE_FAULT_FETCH) {
        printk(", instruction fetch");
    }
    printk(")\n");
}

// Main interrupt service routine handler
void isr_handler(registers_t* regs) {
    uint32_t int_no = regs->int_no;
    console_clear();
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    printk("*** KERNEL PANIC ***\n\n");
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    if (int_no < 32) {
        printk("CPU Exception #%d: %s\n", int_no, exception_messages[int_no]);
        if (regs->err_code != 0) {
            printk("Error Code: 0x%X\n", regs->err_code);
        }
        if (int_no == 14) {
            handle_page_fault(regs);
        }
    } else {
        printk("Unexpected Interrupt #%d\n", int_no);
    }
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    printk("\nRegister Dump:\n");
    printk("  EAX: 0x%08X  EBX: 0x%08X  ECX: 0x%08X  EDX: 0x%08X\n",
           regs->eax, regs->ebx, regs->ecx, regs->edx);
    printk("  ESI: 0x%08X  EDI: 0x%08X  EBP: 0x%08X  ESP: 0x%08X\n",
           regs->esi, regs->edi, regs->ebp, regs->esp);
    printk("  EIP: 0x%08X  EFLAGS: 0x%08X\n", regs->eip, regs->eflags);
    printk("  CS: 0x%04X  DS: 0x%04X  SS: 0x%04X\n",
           regs->cs & 0xFFFF, regs->ds & 0xFFFF, regs->ss & 0xFFFF);
    printk("\nStack Trace (top 8 words):\n");
    uint32_t* stack = (uint32_t*)regs->esp;
    for (int i = 0; i < 8 && (uint32_t)stack < 0x8FFFFFFF; i++) {
        printk("  [ESP+%02d]: 0x%08X\n", i * 4, stack[i]);
    }
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    printk("\nSystem halted. Reset required.\n");
    halt_system();
}

// IRQ dispatcher - handles hardware interrupts
void irq_handler(registers_t* regs) {
    uint32_t int_no = regs->int_no;
    
    // Validate interrupt number range
    if (int_no < 32 || int_no > 47) {
        printk("[IRQ] Invalid interrupt number: %d (expected 32-47)\n", int_no);
        return;
    }
    
    uint32_t irq_no = int_no - 32;  // Convert to IRQ number (0-15)
    
    switch (int_no) {
        case 32: // IRQ 0 - Timer
            timer_handler();  // timer_handler calls pic_send_eoi(0)
            break;
        case 33: // IRQ 1 - Keyboard
            keyboard_handler();  // Handle keyboard input
            pic_send_eoi(1);     // Send EOI for keyboard interrupt
            break;
        default:
            printk("[IRQ] Unhandled hardware interrupt: IRQ %d\n", irq_no);
            // Send EOI for unhandled interrupts
            pic_send_eoi(irq_no);
            break;
    }
}

// General kernel panic function for manual panics
void kernel_panic(const char* message) {
    console_clear();
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    printk("*** KERNEL PANIC ***\n\n");
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    printk("Panic: %s\n\n", message);
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    printk("System halted. Reset required.\n");
    halt_system();
}