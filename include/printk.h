// printk.h - Kernel logging facility header
#ifndef AETHER_PRINTK_H
#define AETHER_PRINTK_H

#include <stdint.h>

// VGA color constants
typedef enum {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
} vga_color_t;

// Console management
void console_clear(void);
void console_set_color(uint8_t color);
uint8_t console_get_color(void);
void console_putchar(char c);
void console_backspace(void);

// Color utility
static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

// Main logging functions
int printk(const char* format, ...) __attribute__((format(printf, 1, 2)));

// Convenience log level functions
void printk_info(const char* format, ...);
void printk_warn(const char* format, ...);
void printk_error(const char* format, ...);

#endif // AETHER_PRINTK_H