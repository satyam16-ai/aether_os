// keyboard.c - PS/2 Keyboard Driver for Aether OS
#include <keyboard.h>
#include <printk.h>
#include <timer.h>
#include <stdint.h>

// US QWERTY keyboard layout
static const char scancode_to_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',        // 0x1D-0x29
    0,    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,          // 0x2A-0x36
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x37-0x43
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,           // 0x44-0x50
    0,    '+', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x51-0x5D
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x5E-0x6A
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x6B-0x77
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0            // 0x78-0x7F
};

// Shifted characters for QWERTY layout
static const char scancode_to_ascii_shift[] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',   // 0x00-0x0E
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  // 0x0F-0x1C
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',        // 0x1D-0x29
    0,    '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,          // 0x2A-0x36
    '*',  0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x37-0x43
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,           // 0x44-0x50
    0,    '+', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x51-0x5D
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x5E-0x6A
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,           // 0x6B-0x77
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0            // 0x78-0x7F
};

// Keyboard state
static struct {
    uint8_t modifiers;
    char input_buffer[256];
    int buffer_head;
    int buffer_tail;
    int buffer_count;
    int input_enabled;
} keyboard_state = {0};

// I/O port functions
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void keyboard_init(void) {
    // Clear any existing data
    keyboard_state.modifiers = 0;
    keyboard_state.buffer_head = 0;
    keyboard_state.buffer_tail = 0;
    keyboard_state.buffer_count = 0;
    keyboard_state.input_enabled = 1;
    
    // Clear keyboard buffer
    while (inb(KB_STATUS_PORT) & KB_STAT_OUTPUT_FULL) {
        inb(KB_DATA_PORT);
    }
    
    // Enable IRQ 1 (keyboard interrupt)
    extern void pic_enable_irq(uint8_t irq);
    pic_enable_irq(1);
    
    printk_info("PS/2 Keyboard driver initialized");
}

static void keyboard_add_to_buffer(char c) {
    if (keyboard_state.buffer_count < 255) {
        keyboard_state.input_buffer[keyboard_state.buffer_head] = c;
        keyboard_state.buffer_head = (keyboard_state.buffer_head + 1) % 256;
        keyboard_state.buffer_count++;
    }
}

void keyboard_handler(void) {
    uint8_t scancode = inb(KB_DATA_PORT);
    
    // Handle key release (bit 7 set)
    if (scancode & KB_KEY_RELEASE) {
        scancode &= ~KB_KEY_RELEASE;
        
        // Handle modifier key releases
        switch (scancode) {
            case KB_LSHIFT:
            case KB_RSHIFT:
                keyboard_state.modifiers &= ~KB_MOD_SHIFT;
                break;
            case KB_LCTRL:
                keyboard_state.modifiers &= ~KB_MOD_CTRL;
                break;
            case KB_LALT:
                keyboard_state.modifiers &= ~KB_MOD_ALT;
                break;
        }
        return;
    }
    
    // Handle modifier key presses
    switch (scancode) {
        case KB_LSHIFT:
        case KB_RSHIFT:
            keyboard_state.modifiers |= KB_MOD_SHIFT;
            return;
        case KB_LCTRL:
            keyboard_state.modifiers |= KB_MOD_CTRL;
            return;
        case KB_LALT:
            keyboard_state.modifiers |= KB_MOD_ALT;
            return;
        case KB_CAPS:
            keyboard_state.modifiers ^= KB_MOD_CAPS;
            return;
    }
    
    // Convert scancode to ASCII
    char ascii = 0;
    if (scancode < 128) {
        if (keyboard_state.modifiers & KB_MOD_SHIFT) {
            ascii = scancode_to_ascii_shift[scancode];
        } else {
            ascii = scancode_to_ascii[scancode];
        }
        
        // Handle caps lock for letters
        if ((keyboard_state.modifiers & KB_MOD_CAPS) && ascii >= 'a' && ascii <= 'z') {
            ascii = ascii - 'a' + 'A';
        } else if ((keyboard_state.modifiers & KB_MOD_CAPS) && ascii >= 'A' && ascii <= 'Z') {
            ascii = ascii - 'A' + 'a';
        }
    }
    
    // Handle special keys
    if (ascii == 0) {
        switch (scancode) {
            case KB_F1:
                printk("\n[F1] System Info: Aether OS v0.1.0\n");
                return;
            case KB_F2:
                printk("\n[F2] Memory Info: Available soon\n");
                return;
            case KB_F3:
                printk("\n[F3] Timer: %d ticks\n", timer_get_ticks());
                return;
        }
        return;
    }
    
    // Add character to input buffer if input is enabled
    if (keyboard_state.input_enabled && ascii != 0) {
        // Echo character to console
        if (ascii == '\b') {
            // Handle backspace
            console_backspace();
        } else if (ascii == '\n') {
            // Handle enter
            console_putchar('\n');
            keyboard_add_to_buffer('\n');
        } else if (ascii >= 32 && ascii <= 126) {
            // Printable characters
            console_putchar(ascii);
            keyboard_add_to_buffer(ascii);
        }
    }
}

char keyboard_getchar(void) {
    if (keyboard_state.buffer_count == 0) {
        return 0;
    }
    
    char c = keyboard_state.input_buffer[keyboard_state.buffer_tail];
    keyboard_state.buffer_tail = (keyboard_state.buffer_tail + 1) % 256;
    keyboard_state.buffer_count--;
    return c;
}

int keyboard_available(void) {
    return keyboard_state.buffer_count;
}

void keyboard_flush(void) {
    keyboard_state.buffer_head = 0;
    keyboard_state.buffer_tail = 0;
    keyboard_state.buffer_count = 0;
}

uint8_t keyboard_get_modifiers(void) {
    return keyboard_state.modifiers;
}

void console_readline(char* buffer, int max_len) {
    int pos = 0;
    
    while (pos < max_len - 1) {
        char c = keyboard_getchar();
        if (c == 0) {
            // No input available, yield CPU
            __asm__ volatile("hlt");
            continue;
        }
        
        if (c == '\n') {
            // Newline already echoed by keyboard handler
            buffer[pos] = '\0';
            return;
        }
        
        if (c == '\b' && pos > 0) {
            // Backspace already handled by keyboard handler
            pos--;
            continue;
        }
        
        if (c >= 32 && c <= 126) {
            // Character already echoed by keyboard handler
            buffer[pos++] = c;
        }
    }
    
    buffer[pos] = '\0';
}

void console_enable_input(void) {
    keyboard_state.input_enabled = 1;
}

void console_disable_input(void) {
    keyboard_state.input_enabled = 0;
}