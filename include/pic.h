#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/**
 * 8259 Programmable Interrupt Controller (PIC) interface
 *
 * The IBM PC uses two 8259 PICs in cascade configuration:
 * - PIC1 (Master): handles IRQ 0-7
 * - PIC2 (Slave): handles IRQ 8-15, connected to IRQ2 of master
 */

/* PIC I/O port addresses */
#define PIC1_COMMAND    0x20    /* Master PIC command port */
#define PIC1_DATA       0x21    /* Master PIC data port */
#define PIC2_COMMAND    0xA0    /* Slave PIC command port */
#define PIC2_DATA       0xA1    /* Slave PIC data port */

/* PIC commands */
#define PIC_EOI         0x20    /* End of Interrupt command */

/* Initialization Command Words (ICW) */
#define ICW1_ICW4       0x01    /* ICW4 needed */
#define ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08    /* Level triggered (edge) mode */
#define ICW1_INIT       0x10    /* Initialization - required! */

#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08    /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C    /* Buffered mode/master */
#define ICW4_SFNM       0x10    /* Special fully nested (not) */

/* IRQ offset mapping - remap IRQs to avoid conflict with CPU exceptions (0-31) */
#define PIC1_OFFSET     0x20    /* IRQ 0-7 mapped to interrupts 32-39 */
#define PIC2_OFFSET     0x28    /* IRQ 8-15 mapped to interrupts 40-47 */

/**
 * Initialize both PICs and remap IRQs
 * IRQs 0-7 are remapped to interrupts 32-39
 * IRQs 8-15 are remapped to interrupts 40-47
 */
void pic_init(void);

/**
 * Send End of Interrupt (EOI) signal to PIC(s)
 * Must be called at the end of IRQ handlers
 * @param irq The IRQ number (0-15)
 */
void pic_send_eoi(uint8_t irq);

/**
 * Set the IRQ mask (disable specific IRQs)
 * @param irq_line The IRQ line to mask (0-15)
 */
void pic_set_mask(uint8_t irq_line);

/**
 * Clear the IRQ mask (enable specific IRQs)
 * @param irq_line The IRQ line to unmask (0-15)
 */
void pic_clear_mask(uint8_t irq_line);

/**
 * Get the combined value of the IRQ request registers (IRR)
 * @return 16-bit value with IRR from both PICs
 */
uint16_t pic_get_irr(void);

/**
 * Get the combined value of the in-service registers (ISR)
 * @return 16-bit value with ISR from both PICs
 */
uint16_t pic_get_isr(void);

#endif /* PIC_H */
