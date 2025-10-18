// shell.c - Simple Interactive Shell for Aether OS
#include <shell.h>
#include <keyboard.h>
#include <printk.h>
#include <memory.h>
#include <timer.h>
#include <stdint.h>

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 16

// String utility functions
static int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

static int strncmp(const char* str1, const char* str2, int n) {
    while (n && *str1 && (*str1 == *str2)) {
        str1++;
        str2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

static char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == c) return (char*)str;
        str++;
    }
    return NULL;
}

static int strlen(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void shell_init(void) {
    printk("\n");
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    printk("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printk("║                          Aether OS Interactive Shell                       ║\n");
    printk("║                                                                            ║\n");
    printk("║  AI-Native Operating System - Phase 3: Keyboard & Memory Management       ║\n");
    printk("║  Type 'help' for available commands                                       ║\n");
    printk("╚════════════════════════════════════════════════════════════════════════════╝\n");
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    printk("\n");
}

static void shell_prompt(void) {
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    printk("aether");
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    printk(":");
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK));
    printk("/");
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    printk("$ ");
}

void shell_run(void) {
    char command_buffer[MAX_COMMAND_LENGTH];
    
    while (1) {
        shell_prompt();
        
        // Read command from keyboard
        console_readline(command_buffer, MAX_COMMAND_LENGTH);
        
        // Process the command
        if (command_buffer[0] != '\0') {
            shell_process_command(command_buffer);
        }
    }
}

void shell_process_command(const char* command) {
    // Skip leading whitespace
    while (*command == ' ' || *command == '\t') command++;
    
    if (*command == '\0') return; // Empty command
    
    // Find end of command word
    const char* args = strchr(command, ' ');
    int cmd_len = args ? (args - command) : (int)strlen(command);
    
    // Skip whitespace in args
    if (args) {
        while (*args == ' ' || *args == '\t') args++;
        if (*args == '\0') args = NULL; // No actual arguments
    }
    
    // Built-in commands
    if (strncmp(command, "help", cmd_len) == 0 && cmd_len == 4) {
        cmd_help();
    } else if (strncmp(command, "clear", cmd_len) == 0 && cmd_len == 5) {
        cmd_clear();
    } else if (strncmp(command, "meminfo", cmd_len) == 0 && cmd_len == 7) {
        cmd_meminfo();
    } else if (strncmp(command, "sysinfo", cmd_len) == 0 && cmd_len == 7) {
        cmd_sysinfo();
    } else if (strncmp(command, "uptime", cmd_len) == 0 && cmd_len == 6) {
        cmd_uptime();
    } else if (strncmp(command, "echo", cmd_len) == 0 && cmd_len == 4) {
        cmd_echo(args);
    } else if (strncmp(command, "test", cmd_len) == 0 && cmd_len == 4) {
        cmd_test(args);
    } else if (strncmp(command, "exit", cmd_len) == 0 && cmd_len == 4) {
        printk("Goodbye! System will halt.\n");
        __asm__ volatile("cli; hlt");
    } else {
        printk("Unknown command: '");
        // Print just the command part
        for (int i = 0; i < cmd_len; i++) {
            console_putchar(command[i]);
        }
        printk("'. Type 'help' for available commands.\n");
    }
}

void cmd_help(void) {
    printk("Available commands:\n");
    printk("  help     - Show this help message\n");
    printk("  clear    - Clear the screen\n");
    printk("  sysinfo  - Display system information\n");
    printk("  meminfo  - Display memory information\n");
    printk("  uptime   - Show system uptime\n");
    printk("  echo     - Echo text to screen\n");
    printk("  test     - Run various tests\n");
    printk("  exit     - Halt the system\n");
    printk("\nFunction Keys:\n");
    printk("  F1       - System info\n");
    printk("  F2       - Memory info\n");
    printk("  F3       - Timer info\n");
}

void cmd_clear(void) {
    console_clear();
}

void cmd_meminfo(void) {
    memory_print_stats();
    
    printk("\nMemory Test - Allocating and freeing blocks:\n");
    void* ptr1 = kmalloc(1024);
    void* ptr2 = kmalloc(2048);
    void* ptr3 = kmalloc(512);
    
    printk("  Allocated: ptr1=%p (1KB), ptr2=%p (2KB), ptr3=%p (512B)\n", 
           ptr1, ptr2, ptr3);
    
    kfree(ptr2);
    printk("  Freed ptr2\n");
    
    void* ptr4 = kmalloc(1500);
    printk("  Allocated: ptr4=%p (1.5KB)\n", ptr4);
    
    kfree(ptr1);
    kfree(ptr3);
    kfree(ptr4);
    printk("  Freed remaining blocks\n");
}

void cmd_sysinfo(void) {
    printk("System Information:\n");
    printk("  OS:          Aether OS v0.1.0\n");
    printk("  Architecture: i386 (32-bit)\n");
    printk("  Bootloader:  GRUB (Multiboot v1)\n");
    printk("  CPU Mode:    Protected Mode\n");
    printk("  Memory:      %u MB total\n", memory_get_total() / (1024 * 1024));
    printk("  Interrupts:  Enabled (PIC initialized)\n");
    printk("  Timer:       PIT at %u Hz\n", timer_get_frequency());
    printk("  Keyboard:    PS/2 driver active\n");
}

void cmd_uptime(void) {
    uint32_t uptime_sec = timer_get_uptime_seconds();
    uint32_t hours = uptime_sec / 3600;
    uint32_t minutes = (uptime_sec % 3600) / 60;
    uint32_t seconds = uptime_sec % 60;
    
    printk("System uptime: %u:%02u:%02u (%u seconds, %u ticks)\n",
           hours, minutes, seconds, uptime_sec, timer_get_ticks());
}

void cmd_echo(const char* args) {
    if (args) {
        printk("%s\n", args);
    } else {
        printk("\n");
    }
}

void cmd_test(const char* args) {
    if (!args) {
        printk("Available tests:\n");
        printk("  test malloc  - Memory allocation test\n");
        printk("  test timer   - Timer functionality test\n");
        printk("  test keys    - Keyboard modifier test\n");
        return;
    }
    
    if (strcmp(args, "malloc") == 0) {
        printk("Memory allocation stress test:\n");
        void* ptrs[10];
        
        for (int i = 0; i < 10; i++) {
            ptrs[i] = kmalloc((i + 1) * 100);
            printk("  malloc(%d bytes) = %p\n", (i + 1) * 100, ptrs[i]);
        }
        
        for (int i = 0; i < 10; i += 2) {
            kfree(ptrs[i]);
            printk("  freed %p\n", ptrs[i]);
        }
        
        for (int i = 1; i < 10; i += 2) {
            kfree(ptrs[i]);
            printk("  freed %p\n", ptrs[i]);
        }
        
        printk("Test completed.\n");
    } else if (strcmp(args, "timer") == 0) {
        printk("Timer test - sleeping for 3 seconds...\n");
        timer_sleep_ms(3000);
        printk("Timer test completed!\n");
    } else if (strcmp(args, "keys") == 0) {
        printk("Keyboard modifier test - press keys to see modifiers:\n");
        printk("Current modifiers: 0x%02x\n", keyboard_get_modifiers());
        printk("(Press any key to continue)\n");
        keyboard_getchar(); // Wait for keypress
    } else {
        printk("Unknown test: %s\n", args);
    }
}