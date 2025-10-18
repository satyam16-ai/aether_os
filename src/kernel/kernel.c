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
#include <paging.h>

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
    
    // Initialize Paging (Virtual Memory) - Phase 4 Step 1
    paging_init();
    
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
    printk("  [DONE] Paging - Virtual Memory (initialized, not yet enabled)\n");
    printk("  [DONE] Keyboard - PS/2 Driver\n");
    printk("  [TODO] Paging Enable - Activate virtual memory\n");
    printk("  [TODO] Scheduler - Process Management\n");
    printk("  [TODO] Syscalls - System Call Interface\n");
    printk("  [TODO] VFS - Virtual File System\n");
    printk("  [TODO] Drivers - Hardware Abstraction\n");
    
    printk_warn("Sentinel AI integration hooks planned for Phase 6");
    
    printk("\n");
    printk_info("Kernel initialization complete. All subsystems ready.");
    printk_info("Phase 4 Step 2: Virtual Memory Ready (use 'paging enable')");
    
    // Brief delay to ensure hardware is ready
    printk("\nWaiting for hardware to settle...\n");
    timer_sleep_ms(500);
    
    shell_init();
    shell_run(); // This runs forever, handling user input
}
