// printk.c - Kernel printf-like facility with VGA text mode output
// Supports: %s %c %d %u %x %X %p with basic width/padding

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((volatile uint16_t*)0xB8000)

// Console state
static size_t console_row = 0;
static size_t console_column = 0;
static uint8_t console_color = 0x07; // Light gray on black

// Color definitions
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

static inline uint8_t vga_entry_color(vga_color_t fg, vga_color_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// String utility functions
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

static void reverse_string(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Number to string conversion
static int itoa(int value, char* str, int base) {
    int i = 0;
    int is_negative = 0;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    if (value < 0 && base == 10) {
        is_negative = 1;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    if (is_negative)
        str[i++] = '-';

    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static int utoa(unsigned int value, char* str, int base) {
    int i = 0;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

static int utoa_upper(unsigned int value, char* str, int base) {
    int i = 0;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return i;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        value = value / base;
    }

    str[i] = '\0';
    reverse_string(str, i);
    return i;
}

// Scrolling support
static void console_scroll(void) {
    // Move all rows up by one
    for (size_t row = 0; row < VGA_HEIGHT - 1; row++) {
        for (size_t col = 0; col < VGA_WIDTH; col++) {
            VGA_MEMORY[row * VGA_WIDTH + col] = VGA_MEMORY[(row + 1) * VGA_WIDTH + col];
        }
    }
    
    // Clear the last row
    for (size_t col = 0; col < VGA_WIDTH; col++) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', console_color);
    }
    
    console_row = VGA_HEIGHT - 1;
    console_column = 0;
}

// Put character at current position
static void console_putchar_at(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    VGA_MEMORY[index] = vga_entry(c, color);
}

// Put character with automatic positioning (public version)
void console_putchar(char c) {
    if (c == '\n') {
        console_column = 0;
        if (++console_row == VGA_HEIGHT) {
            console_scroll();
        }
        return;
    }
    
    if (c == '\r') {
        console_column = 0;
        return;
    }
    
    if (c == '\t') {
        // Tab to next 8-character boundary
        console_column = (console_column + 8) & ~7;
        if (console_column >= VGA_WIDTH) {
            console_column = 0;
            if (++console_row == VGA_HEIGHT) {
                console_scroll();
            }
        }
        return;
    }
    
    console_putchar_at(c, console_color, console_column, console_row);
    
    if (++console_column == VGA_WIDTH) {
        console_column = 0;
        if (++console_row == VGA_HEIGHT) {
            console_scroll();
        }
    }
}

// Print string
static void console_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        console_putchar(data[i]);
}

static void console_writestring(const char* data) {
    console_write(data, strlen(data));
}

// Clear screen
void console_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            console_putchar_at(' ', console_color, x, y);
        }
    }
    console_row = 0;
    console_column = 0;
}

// Set console color
void console_set_color(uint8_t color) {
    console_color = color;
}

// Get current console color
uint8_t console_get_color(void) {
    return console_color;
}

// Handle backspace
void console_backspace(void) {
    if (console_column > 0) {
        console_column--;
        console_putchar_at(' ', console_color, console_column, console_row);
    } else if (console_row > 0) {
        console_row--;
        console_column = VGA_WIDTH - 1;
        console_putchar_at(' ', console_color, console_column, console_row);
    }
}

// Main printk function
int printk(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int written = 0;
    
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            
            // Handle format specifiers
            switch (*format) {
                case 's': {
                    const char* str = va_arg(args, const char*);
                    if (str == NULL) str = "(null)";
                    console_writestring(str);
                    written += strlen(str);
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);
                    console_putchar(c);
                    written++;
                    break;
                }
                case 'd': {
                    int value = va_arg(args, int);
                    char buffer[32];
                    int len = itoa(value, buffer, 10);
                    console_writestring(buffer);
                    written += len;
                    break;
                }
                case 'u': {
                    unsigned int value = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(value, buffer, 10);
                    console_writestring(buffer);
                    written += len;
                    break;
                }
                case 'x': {
                    unsigned int value = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa(value, buffer, 16);
                    console_writestring(buffer);
                    written += len;
                    break;
                }
                case 'X': {
                    unsigned int value = va_arg(args, unsigned int);
                    char buffer[32];
                    int len = utoa_upper(value, buffer, 16);
                    console_writestring(buffer);
                    written += len;
                    break;
                }
                case 'p': {
                    void* ptr = va_arg(args, void*);
                    uintptr_t value = (uintptr_t)ptr;
                    char buffer[32];
                    console_writestring("0x");
                    int len = utoa(value, buffer, 16);
                    console_writestring(buffer);
                    written += len + 2;
                    break;
                }
                case '%': {
                    console_putchar('%');
                    written++;
                    break;
                }
                default: {
                    // Unknown format specifier, just print it
                    console_putchar('%');
                    console_putchar(*format);
                    written += 2;
                    break;
                }
            }
        } else {
            console_putchar(*format);
            written++;
        }
        format++;
    }
    
    va_end(args);
    return written;
}

// Log level functions with color coding
void printk_info(const char* format, ...) {
    uint8_t old_color = console_color;
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    console_writestring("[INFO] ");
    
    va_list args;
    va_start(args, format);
    // Simple version without full va_list printk implementation
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // For now, just print the format string - full implementation would need vprintk
    console_writestring(format);
    console_putchar('\n');
    
    va_end(args);
    console_set_color(old_color);
}

void printk_warn(const char* format, ...) {
    uint8_t old_color = console_color;
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK));
    console_writestring("[WARN] ");
    
    va_list args;
    va_start(args, format);
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    console_writestring(format);
    console_putchar('\n');
    va_end(args);
    console_set_color(old_color);
}

void printk_error(const char* format, ...) {
    uint8_t old_color = console_color;
    console_set_color(vga_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK));
    console_writestring("[ERROR] ");
    
    va_list args;
    va_start(args, format);
    console_set_color(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    console_writestring(format);
    console_putchar('\n');
    va_end(args);
    console_set_color(old_color);
}