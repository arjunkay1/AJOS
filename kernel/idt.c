/**
 * AJOS Interrupt Descriptor Table (IDT) Implementation
 * Handles hardware interrupts (IRQs) and CPU exceptions
 */

#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/io.h"
#include "../include/vga.h"

/* Forward declaration for keyboard handler if available */
extern void keyboard_handler(void) __attribute__((weak));

/* IDT with 256 entries */
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

/* Exception messages for debugging */
static const char *exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

/**
 * Set an IDT entry
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

/**
 * Load the IDT register using lidt instruction
 */
static void idt_load(void) {
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

/**
 * Install ISR handlers for CPU exceptions (0-31)
 */
static void isr_install(void) {
    idt_set_gate(0,  (uint32_t)isr0,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_INTERRUPT_GATE);
}

/**
 * Install IRQ handlers (32-47)
 */
static void irq_install(void) {
    idt_set_gate(32, (uint32_t)irq0,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(33, (uint32_t)irq1,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(34, (uint32_t)irq2,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(35, (uint32_t)irq3,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(36, (uint32_t)irq4,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(37, (uint32_t)irq5,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(38, (uint32_t)irq6,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(39, (uint32_t)irq7,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(40, (uint32_t)irq8,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(41, (uint32_t)irq9,  0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_INTERRUPT_GATE);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_INTERRUPT_GATE);
}

/**
 * Initialize IDT and PIC
 */
void idt_init(void) {
    /* Set up IDT pointer */
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (uint32_t)&idt;

    /* Clear all IDT entries */
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Initialize the PIC and remap IRQs */
    pic_init();

    /* Install exception handlers (ISR 0-31) */
    isr_install();

    /* Install IRQ handlers (32-47) */
    irq_install();

    /* Load the IDT */
    idt_load();

    /* Enable interrupts */
    __asm__ volatile ("sti");
}

/**
 * Helper function to print a number in hex
 */
static void print_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[11] = "0x00000000";

    for (int i = 9; i >= 2; i--) {
        buffer[i] = hex_chars[n & 0xF];
        n >>= 4;
    }
    vga_print(buffer);
}

/**
 * ISR handler - called from assembly stub for CPU exceptions
 */
void isr_handler(registers_t *regs) {
    /* Print exception info */
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_print("\n*** EXCEPTION: ");

    if (regs->int_no < 32) {
        vga_print(exception_messages[regs->int_no]);
    } else {
        vga_print("Unknown Exception");
    }

    vga_print(" ***\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    vga_print("Interrupt: ");
    print_hex(regs->int_no);
    vga_print("  Error Code: ");
    print_hex(regs->err_code);
    vga_print("\n");

    vga_print("EIP: ");
    print_hex(regs->eip);
    vga_print("  CS: ");
    print_hex(regs->cs);
    vga_print("  EFLAGS: ");
    print_hex(regs->eflags);
    vga_print("\n");

    vga_print("EAX: ");
    print_hex(regs->eax);
    vga_print("  EBX: ");
    print_hex(regs->ebx);
    vga_print("  ECX: ");
    print_hex(regs->ecx);
    vga_print("  EDX: ");
    print_hex(regs->edx);
    vga_print("\n");

    /* Halt the system on exception */
    vga_print("System halted.\n");
    __asm__ volatile ("cli; hlt");
}

/**
 * IRQ handler - called from assembly stub for hardware interrupts
 */
void irq_handler(registers_t *regs) {
    /* Calculate the IRQ number (interrupt 32-47 = IRQ 0-15) */
    uint8_t irq = regs->int_no - 32;

    /* Dispatch to specific handlers */
    switch (irq) {
        case 0:  /* Timer (PIT) - IRQ0 */
            /* Timer tick handler would go here */
            break;

        case 1:  /* Keyboard - IRQ1 */
            if (keyboard_handler) {
                keyboard_handler();
            }
            break;

        case 2:  /* Cascade - IRQ2 */
            /* Used internally by the PICs */
            break;

        case 12: /* PS/2 Mouse - IRQ12 */
            /* Mouse handler would go here */
            break;

        default:
            /* Unhandled IRQ - just acknowledge it */
            break;
    }

    /* Send End of Interrupt to PIC */
    pic_send_eoi(irq);
}
