#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

/* Keyboard I/O ports */
#define KEYBOARD_DATA_PORT   0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Keyboard buffer size */
#define KEYBOARD_BUFFER_SIZE 256

/* Special key codes */
#define KEY_BACKSPACE 0x08
#define KEY_ENTER     0x0A
#define KEY_TAB       0x09
#define KEY_ESCAPE    0x1B

/* Arrow key codes (use values > 127 to avoid ASCII conflict) */
#define KEY_UP        0x80
#define KEY_DOWN      0x81
#define KEY_LEFT      0x82
#define KEY_RIGHT     0x83

/* Function declarations */

/**
 * Initialize the keyboard driver
 * Must be called after IDT and PIC are set up
 */
void keyboard_init(void);

/**
 * Keyboard interrupt handler (IRQ1 / Interrupt 33)
 * Called from the interrupt handler in idt.c
 */
void keyboard_handler(void);

/**
 * Read one character from keyboard (blocking)
 * Waits until a key is pressed and returns the ASCII character
 */
char keyboard_getchar(void);

/**
 * Check if there are characters available in the buffer
 * Returns 1 if buffer has data, 0 otherwise
 */
int keyboard_has_data(void);

/**
 * Get character from buffer without blocking
 * Returns the character or 0 if buffer is empty
 */
char keyboard_getchar_nonblocking(void);

#endif /* KEYBOARD_H */
