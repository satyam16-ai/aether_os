#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// PS/2 Keyboard Controller Ports
#define KB_DATA_PORT     0x60
#define KB_STATUS_PORT   0x64
#define KB_COMMAND_PORT  0x64

// Status Register Bits
#define KB_STAT_OUTPUT_FULL  0x01
#define KB_STAT_INPUT_FULL   0x02

// Special Key Codes
#define KB_ESC          0x01
#define KB_BACKSPACE    0x0E
#define KB_TAB          0x0F
#define KB_ENTER        0x1C
#define KB_LCTRL        0x1D
#define KB_LSHIFT       0x2A
#define KB_RSHIFT       0x36
#define KB_LALT         0x38
#define KB_SPACE        0x39
#define KB_CAPS         0x3A
#define KB_F1           0x3B
#define KB_F2           0x3C
#define KB_F3           0x3D
#define KB_F4           0x3E
#define KB_F5           0x3F
#define KB_F6           0x40
#define KB_F7           0x41
#define KB_F8           0x42
#define KB_F9           0x43
#define KB_F10          0x44

// Key release flag (bit 7 set)
#define KB_KEY_RELEASE  0x80

// Keyboard modifier flags
#define KB_MOD_SHIFT    0x01
#define KB_MOD_CTRL     0x02
#define KB_MOD_ALT      0x04
#define KB_MOD_CAPS     0x08

// Function prototypes
void keyboard_init(void);
void keyboard_handler(void);
char keyboard_getchar(void);
int keyboard_available(void);
void keyboard_flush(void);
uint8_t keyboard_get_modifiers(void);

// Console input functions
void console_readline(char* buffer, int max_len);
void console_enable_input(void);
void console_disable_input(void);

#endif // KEYBOARD_H