#include "../include/vga.h"

/* Static variables for cursor position and color */
static uint16_t* vga_buffer = (uint16_t*)VGA_BUFFER_ADDR;
static uint8_t vga_row = 0;
static uint8_t vga_col = 0;
static uint8_t vga_current_color = 0;

/* Port I/O for cursor control */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/* Update hardware cursor position */
static void vga_update_cursor(void) {
    uint16_t pos = vga_row * VGA_WIDTH + vga_col;

    /* CRT Controller registers */
    /* 0x3D4 = address register, 0x3D5 = data register */

    /* Set cursor location low byte */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    /* Set cursor location high byte */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

/* Helper function to create VGA entry */
static inline uint16_t vga_entry(char c, uint8_t color) {
    return (uint16_t)c | ((uint16_t)color << 8);
}

/* Helper function to create color attribute */
static inline uint8_t vga_make_color(vga_color_t fg, vga_color_t bg) {
    return (uint8_t)fg | ((uint8_t)bg << 4);
}

/* Scroll the screen up by one line */
static void vga_scroll(void) {
    /* Move all lines up by one */
    for (int row = 0; row < VGA_HEIGHT - 1; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buffer[row * VGA_WIDTH + col] = vga_buffer[(row + 1) * VGA_WIDTH + col];
        }
    }

    /* Clear the last line */
    for (int col = 0; col < VGA_WIDTH; col++) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + col] = vga_entry(' ', vga_current_color);
    }
}

/* Initialize VGA driver */
void vga_init(void) {
    vga_row = 0;
    vga_col = 0;
    vga_current_color = vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_update_cursor();
}

/* Clear the screen */
void vga_clear(void) {
    for (int row = 0; row < VGA_HEIGHT; row++) {
        for (int col = 0; col < VGA_WIDTH; col++) {
            vga_buffer[row * VGA_WIDTH + col] = vga_entry(' ', vga_current_color);
        }
    }
    vga_row = 0;
    vga_col = 0;
    vga_update_cursor();
}

/* Print a single character */
void vga_putchar(char c) {
    /* Handle newline */
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    }
    /* Handle backspace */
    else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
        } else if (vga_row > 0) {
            vga_row--;
            vga_col = VGA_WIDTH - 1;
        }
    }
    /* Handle carriage return */
    else if (c == '\r') {
        vga_col = 0;
    }
    /* Handle tab */
    else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;  /* Align to next 8-column boundary */
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }
    else {
        /* Write character to VGA buffer */
        vga_buffer[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_current_color);
        vga_col++;

        /* Wrap to next line if needed */
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }

    /* Scroll if we've reached the bottom */
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
        vga_row = VGA_HEIGHT - 1;
    }

    /* Update hardware cursor position */
    vga_update_cursor();
}

/* Print a null-terminated string */
void vga_print(const char* str) {
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

/* Set the current text color */
void vga_set_color(vga_color_t fg, vga_color_t bg) {
    vga_current_color = vga_make_color(fg, bg);
}
