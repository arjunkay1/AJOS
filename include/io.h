#ifndef IO_H
#define IO_H

#include <stdint.h>

/**
 * Port I/O functions for x86
 * Used to communicate with hardware devices like the PIC
 */

/**
 * Write a byte to the specified I/O port
 * @param port The I/O port address
 * @param value The byte value to write
 */
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Read a byte from the specified I/O port
 * @param port The I/O port address
 * @return The byte value read from the port
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

/**
 * Wait a small amount of time (approximately 1-4 microseconds)
 * Used for I/O operations that require a delay (e.g., PIC initialization)
 * Writes to an unused port (0x80) which takes time to complete
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}

/**
 * Write a word (16-bit) to the specified I/O port
 * @param port The I/O port address
 * @param value The word value to write
 */
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

/**
 * Read a word (16-bit) from the specified I/O port
 * @param port The I/O port address
 * @return The word value read from the port
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

#endif /* IO_H */
