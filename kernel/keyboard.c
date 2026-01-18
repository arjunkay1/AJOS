/**
 * AJOS PS/2 Keyboard Driver
 * Handles keyboard input via IRQ1 (Interrupt 33)
 * Uses Scancode Set 1 (US QWERTY layout)
 */

#include "../include/keyboard.h"
#include "../include/io.h"
#include "../include/pic.h"

/* Circular buffer for keystrokes */
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint8_t buffer_head = 0;
static volatile uint8_t buffer_tail = 0;

/* Shift key state */
static volatile uint8_t shift_pressed = 0;
static volatile uint8_t caps_lock = 0;

/* Scancode to ASCII lookup table (lowercase, Scancode Set 1) */
static const char scancode_to_ascii[128] = {
    0,    0x1B, '1',  '2',  '3',  '4',  '5',  '6',   /* 0x00 - 0x07 */
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',  /* 0x08 - 0x0F */
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',   /* 0x10 - 0x17 */
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',   /* 0x18 - 0x1F */
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',   /* 0x20 - 0x27 */
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',   /* 0x28 - 0x2F */
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',   /* 0x30 - 0x37 */
    0,    ' ',  0,    0,    0,    0,    0,    0,     /* 0x38 - 0x3F */
    0,    0,    0,    0,    0,    0,    0,    '7',   /* 0x40 - 0x47 */
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',   /* 0x48 - 0x4F */
    '2',  '3',  '0',  '.',  0,    0,    0,    0,     /* 0x50 - 0x57 */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x58 - 0x5F */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x60 - 0x67 */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x68 - 0x6F */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x70 - 0x77 */
    0,    0,    0,    0,    0,    0,    0,    0      /* 0x78 - 0x7F */
};

/* Scancode to ASCII lookup table (shifted/uppercase) */
static const char scancode_to_ascii_shift[128] = {
    0,    0x1B, '!',  '@',  '#',  '$',  '%',  '^',   /* 0x00 - 0x07 */
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',  /* 0x08 - 0x0F */
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',   /* 0x10 - 0x17 */
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',   /* 0x18 - 0x1F */
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',   /* 0x20 - 0x27 */
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',   /* 0x28 - 0x2F */
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',   /* 0x30 - 0x37 */
    0,    ' ',  0,    0,    0,    0,    0,    0,     /* 0x38 - 0x3F */
    0,    0,    0,    0,    0,    0,    0,    '7',   /* 0x40 - 0x47 */
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',   /* 0x48 - 0x4F */
    '2',  '3',  '0',  '.',  0,    0,    0,    0,     /* 0x50 - 0x57 */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x58 - 0x5F */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x60 - 0x67 */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x68 - 0x6F */
    0,    0,    0,    0,    0,    0,    0,    0,     /* 0x70 - 0x77 */
    0,    0,    0,    0,    0,    0,    0,    0      /* 0x78 - 0x7F */
};

/* Scancode constants */
#define SCANCODE_LEFT_SHIFT_PRESS    0x2A
#define SCANCODE_LEFT_SHIFT_RELEASE  0xAA
#define SCANCODE_RIGHT_SHIFT_PRESS   0x36
#define SCANCODE_RIGHT_SHIFT_RELEASE 0xB6
#define SCANCODE_CAPS_LOCK           0x3A
#define SCANCODE_CTRL                0x1D
#define SCANCODE_ALT                 0x38
#define SCANCODE_EXTENDED            0xE0

/* Extended key scancodes (after 0xE0 prefix) */
#define SCANCODE_EXT_UP              0x48
#define SCANCODE_EXT_DOWN            0x50
#define SCANCODE_EXT_LEFT            0x4B
#define SCANCODE_EXT_RIGHT           0x4D

/* Extended scancode state */
static volatile uint8_t extended_scancode = 0;

/**
 * Add a character to the keyboard buffer
 */
static void buffer_put(char c) {
    uint8_t next = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;

    /* Don't overwrite if buffer is full */
    if (next != buffer_tail) {
        keyboard_buffer[buffer_head] = c;
        buffer_head = next;
    }
}

/**
 * Get a character from the keyboard buffer
 * Returns 0 if buffer is empty
 */
static char buffer_get(void) {
    if (buffer_head == buffer_tail) {
        return 0;  /* Buffer empty */
    }

    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

/**
 * Check if buffer has data
 */
static int buffer_has_data(void) {
    return buffer_head != buffer_tail;
}

/**
 * Convert scancode to ASCII character
 */
static char scancode_to_char(uint8_t scancode) {
    /* Ignore key releases (bit 7 set) for character conversion */
    if (scancode >= 0x80) {
        return 0;
    }

    char c;

    /* Check if shift is pressed for special characters */
    if (shift_pressed) {
        c = scancode_to_ascii_shift[scancode];
    } else {
        c = scancode_to_ascii[scancode];
    }

    /* Handle caps lock for letters only */
    if (caps_lock && c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    } else if (caps_lock && c >= 'A' && c <= 'Z') {
        c = c - 'A' + 'a';
    }

    return c;
}

/**
 * Initialize the keyboard driver
 */
void keyboard_init(void) {
    /* Clear the buffer */
    buffer_head = 0;
    buffer_tail = 0;
    shift_pressed = 0;
    caps_lock = 0;

    /* Flush keyboard buffer by reading any pending data */
    while (inb(KEYBOARD_STATUS_PORT) & 0x01) {
        inb(KEYBOARD_DATA_PORT);
    }

    /* Enable IRQ1 (keyboard interrupt) by clearing mask */
    pic_clear_mask(1);
}

/**
 * Keyboard interrupt handler (IRQ1)
 * Called from IRQ handler in idt.c
 */
void keyboard_handler(void) {
    uint8_t scancode;

    /* Read scancode from keyboard data port */
    scancode = inb(KEYBOARD_DATA_PORT);

    /* Handle extended scancode prefix */
    if (scancode == SCANCODE_EXTENDED) {
        extended_scancode = 1;
        return;
    }

    /* Handle extended scancodes (arrow keys, etc.) */
    if (extended_scancode) {
        extended_scancode = 0;

        /* Ignore key releases */
        if (scancode >= 0x80) {
            return;
        }

        /* Map extended scancodes to special key values */
        switch (scancode) {
            case SCANCODE_EXT_UP:
                buffer_put(KEY_UP);
                break;
            case SCANCODE_EXT_DOWN:
                buffer_put(KEY_DOWN);
                break;
            case SCANCODE_EXT_LEFT:
                buffer_put(KEY_LEFT);
                break;
            case SCANCODE_EXT_RIGHT:
                buffer_put(KEY_RIGHT);
                break;
        }
        return;
    }

    /* Handle shift key press/release */
    if (scancode == SCANCODE_LEFT_SHIFT_PRESS ||
        scancode == SCANCODE_RIGHT_SHIFT_PRESS) {
        shift_pressed = 1;
        return;
    }

    if (scancode == SCANCODE_LEFT_SHIFT_RELEASE ||
        scancode == SCANCODE_RIGHT_SHIFT_RELEASE) {
        shift_pressed = 0;
        return;
    }

    /* Handle caps lock (toggle on press only) */
    if (scancode == SCANCODE_CAPS_LOCK) {
        caps_lock = !caps_lock;
        return;
    }

    /* Ignore key releases for other keys */
    if (scancode >= 0x80) {
        return;
    }

    /* Convert scancode to ASCII */
    char c = scancode_to_char(scancode);

    /* Add valid characters to buffer */
    if (c != 0) {
        buffer_put(c);
    }
}

/**
 * Read one character from keyboard (blocking)
 */
char keyboard_getchar(void) {
    /* Wait until buffer has data */
    while (!buffer_has_data()) {
        /* Halt CPU until next interrupt to save power */
        __asm__ volatile ("hlt");
    }

    return buffer_get();
}

/**
 * Check if there are characters available
 */
int keyboard_has_data(void) {
    return buffer_has_data();
}

/**
 * Get character without blocking
 */
char keyboard_getchar_nonblocking(void) {
    return buffer_get();
}
