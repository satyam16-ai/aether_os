// kernel.c - Initial Aether OS kernel stub
// Freestanding: avoid using any hosted library features.
#include <stdint.h>
#include <stddef.h>
#include <printk.h>
#include <gdt.h>
#include <idt.h>
#include <pic.h>
#include <timer.h>
#include <memory.h>
#include <keyboard.h>
#include <shell.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void kmain(void) {
    // Clear screen and set up console
    console_clear();
    
    // Boot banner with colors
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    printk("    _   _   _   _   _   _   _   _  \n");
    printk("   / \\ / \\ / \\ / \\ / \\ / \\ / \\ / \\ \n");
    printk("  ( A )e( t )h( e )r( O )S(  ) \n");
    printk("   \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \\_/ \n");
    
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    printk("\nAether OS Kernel v0.1.0 - Booted via GRUB\n");
    printk("AI-Native Operating System - Phase 0 Complete\n\n");
    
    // Demonstrate printk format specifiers
    printk("printk Test Suite:\n");
    printk("  String: %s\n", "Hello, Aether!");
    printk("  Character: %c\n", 'A');
    printk("  Decimal: %d\n", -12345);
    printk("  Unsigned: %u\n", 4294967295U);
    printk("  Hex (lower): 0x%x\n", 0xDEADBEEF);
    printk("  Hex (upper): 0x%X\n", 0xCAFEBABE);
    printk("  Pointer: %p\n", kmain);
    printk("  Literal %%: 100%%\n\n");
    
    // Phase 1: Initialize core kernel subsystems
    printk_info("Phase 1: Kernel Foundation Initialization");
    
    // Initialize GDT (Global Descriptor Table)
    gdt_init();
    
    // Initialize IDT (Interrupt Descriptor Table) 
    idt_init();
    
    // Initialize PIC (Programmable Interrupt Controller)
    pic_init();
    
    // Initialize Timer (100 Hz = 10ms intervals)
    timer_init(100);
    
    // Initialize Memory Manager
    memory_init();
    
    // Initialize Keyboard Driver
    keyboard_init();
    
    // Enable interrupts
    printk_info("Enabling hardware interrupts");
    __asm__ volatile("sti");
    
    // Subsystem initialization status
    printk("\nSubsystem Status:\n");
    printk("  [DONE] GDT - Global Descriptor Table\n");
    printk("  [DONE] IDT - Interrupt Descriptor Table (exceptions + IRQs)\n");
    printk("  [DONE] PIC - Programmable Interrupt Controller\n");
    printk("  [DONE] PIT - Programmable Interval Timer (100 Hz)\n");
    printk("  [DONE] Memory - Kernel Heap Allocator (4MB)\n");
    printk("  [DONE] Keyboard - PS/2 Driver\n");
    printk("  [TODO] Paging - Virtual Memory Management\n");
    printk("  [TODO] Scheduler - Process Management\n");
    printk("  [TODO] Syscalls - System Call Interface\n");
    printk("  [TODO] VFS - Virtual File System\n");
    printk("  [TODO] Drivers - Hardware Abstraction\n");
    
    printk_warn("Sentinel AI integration hooks planned for Phase 6");
    
    // Memory info placeholder
    printk("\nMemory Layout (current):\n");
    printk("  Kernel loaded at: %p\n", kmain);
    printk("  Stack pointer: %p\n", __builtin_frame_address(0));
    
    // Simple port write (status LED / POST code) - demonstration only
    outb(0x80, 0x55);
    printk_info("POST code 0x55 written to port 0x80");
    
    printk("\nTimer Test:\n");
    printk_info("Hardware interrupts enabled. Timer ticking at 100 Hz.");
    printk("  Watch for timer messages every second...\n");
    
    // Demonstrate timer sleep function
    printk("Testing sleep function: sleeping for 2 seconds...\n");
    timer_sleep_ms(2000);
    printk("Wake up! Sleep test completed.\n\n");
    
    printk("Kernel initialization complete. All subsystems ready.\n");
    printk_info("Phase 3 Complete: Interactive Shell with Memory Management");
    
    // Initialize and run the interactive shell
    printk("\nStarting interactive shell...\n");
    timer_sleep_ms(1000); // Brief pause for effect
    
    shell_init();
    shell_run(); // This runs forever, handling user input
}
