#include "../include/pic.h"
#include "../include/io.h"

/**
 * Initialize both 8259 PICs
 *
 * The initialization sequence consists of:
 * 1. ICW1: Start initialization in cascade mode
 * 2. ICW2: Set vector offset (where IRQs start in IDT)
 * 3. ICW3: Configure master/slave relationship
 * 4. ICW4: Set 8086 mode
 */
void pic_init(void) {
    uint8_t mask1, mask2;

    /* Save current interrupt masks */
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    /* ICW1: Start initialization sequence (cascade mode) */
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    /* ICW2: Set vector offsets */
    outb(PIC1_DATA, PIC1_OFFSET);   /* Master PIC: IRQ 0-7 -> INT 32-39 */
    io_wait();
    outb(PIC2_DATA, PIC2_OFFSET);   /* Slave PIC: IRQ 8-15 -> INT 40-47 */
    io_wait();

    /* ICW3: Configure cascade (master/slave wiring) */
    outb(PIC1_DATA, 0x04);          /* Tell Master PIC that Slave is at IRQ2 (0000 0100) */
    io_wait();
    outb(PIC2_DATA, 0x02);          /* Tell Slave PIC its cascade identity (0000 0010) */
    io_wait();

    /* ICW4: Set 8086 mode */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Restore saved masks (or set to 0x00 to enable all IRQs) */
    /* For now, mask all IRQs except cascade (IRQ2) */
    outb(PIC1_DATA, 0xFB);  /* 1111 1011 - only IRQ2 enabled (cascade) */
    outb(PIC2_DATA, 0xFF);  /* 1111 1111 - all slave IRQs masked */
}

/**
 * Send End of Interrupt (EOI) signal
 * For IRQs 8-15 (slave), we need to send EOI to both PICs
 */
void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        /* IRQ came from slave PIC, send EOI to slave first */
        outb(PIC2_COMMAND, PIC_EOI);
    }
    /* Always send EOI to master PIC */
    outb(PIC1_COMMAND, PIC_EOI);
}

/**
 * Set (disable) an IRQ line by setting its mask bit
 */
void pic_set_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) | (1 << irq_line);
    outb(port, value);
}

/**
 * Clear (enable) an IRQ line by clearing its mask bit
 */
void pic_clear_mask(uint8_t irq_line) {
    uint16_t port;
    uint8_t value;

    if (irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }
    value = inb(port) & ~(1 << irq_line);
    outb(port, value);
}

/**
 * Helper function to get the register value from PIC
 */
static uint16_t pic_get_irq_reg(int ocw3) {
    /* OCW3 to PIC CMD to get the register values.
     * PIC2 is chained, and represents IRQs 8-15.
     * PIC1 is master, and represents IRQs 0-7. */
    outb(PIC1_COMMAND, ocw3);
    outb(PIC2_COMMAND, ocw3);
    return (inb(PIC2_COMMAND) << 8) | inb(PIC1_COMMAND);
}

/**
 * Get the combined IRQ Request Register (IRR)
 * Returns IRQs that have been raised but not yet serviced
 */
uint16_t pic_get_irr(void) {
    return pic_get_irq_reg(0x0A);  /* OCW3 irr */
}

/**
 * Get the combined In-Service Register (ISR)
 * Returns IRQs that are currently being serviced
 */
uint16_t pic_get_isr(void) {
    return pic_get_irq_reg(0x0B);  /* OCW3 isr */
}
