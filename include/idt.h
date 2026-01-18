#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/**
 * Interrupt Descriptor Table (IDT) structures and declarations
 *
 * The IDT is used by the processor to determine the correct response
 * to interrupts and exceptions.
 */

/* IDT Entry (Gate Descriptor) - 8 bytes */
struct idt_entry {
    uint16_t base_low;      /* Lower 16 bits of handler address */
    uint16_t selector;      /* Kernel code segment selector */
    uint8_t  zero;          /* Always zero */
    uint8_t  flags;         /* Type and attributes */
    uint16_t base_high;     /* Upper 16 bits of handler address */
} __attribute__((packed));

typedef struct idt_entry idt_entry_t;

/* IDT Pointer - used by lidt instruction */
struct idt_ptr {
    uint16_t limit;         /* Size of IDT - 1 */
    uint32_t base;          /* Base address of IDT */
} __attribute__((packed));

typedef struct idt_ptr idt_ptr_t;

/* IDT flags */
#define IDT_FLAG_PRESENT    0x80    /* Present bit - must be 1 for valid entries */
#define IDT_FLAG_DPL0       0x00    /* Descriptor Privilege Level 0 (kernel) */
#define IDT_FLAG_DPL3       0x60    /* Descriptor Privilege Level 3 (user) */
#define IDT_FLAG_INT_GATE   0x0E    /* 32-bit Interrupt Gate */
#define IDT_FLAG_TRAP_GATE  0x0F    /* 32-bit Trap Gate */
#define IDT_FLAG_TASK_GATE  0x05    /* Task Gate */

/* Standard kernel interrupt gate flags */
#define IDT_INTERRUPT_GATE  (IDT_FLAG_PRESENT | IDT_FLAG_DPL0 | IDT_FLAG_INT_GATE)
#define IDT_TRAP_GATE       (IDT_FLAG_PRESENT | IDT_FLAG_DPL0 | IDT_FLAG_TRAP_GATE)

/* Number of IDT entries */
#define IDT_ENTRIES         256

/**
 * Initialize the Interrupt Descriptor Table
 * Sets up exception handlers (0-31) and IRQ handlers (32-47)
 * and loads the IDT
 */
void idt_init(void);

/**
 * Set an IDT gate entry
 * @param num The interrupt number (0-255)
 * @param base The address of the interrupt handler
 * @param selector The code segment selector
 * @param flags The type and attribute flags
 */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

/* Registers structure pushed by ISR/IRQ stub */
typedef struct {
    uint32_t ds;                                     /* Data segment selector */
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; /* Pushed by pusha */
    uint32_t int_no, err_code;                       /* Interrupt number and error code */
    uint32_t eip, cs, eflags, useresp, ss;           /* Pushed by processor */
} registers_t;

/**
 * Exception handler (called from assembly stub)
 * @param regs Pointer to saved register state
 */
void isr_handler(registers_t *regs);

/**
 * IRQ handler (called from assembly stub)
 * @param regs Pointer to saved register state
 */
void irq_handler(registers_t *regs);

/* ISR (Interrupt Service Routine) declarations - CPU exceptions 0-31 */
extern void isr0(void);     /* Division By Zero */
extern void isr1(void);     /* Debug */
extern void isr2(void);     /* Non Maskable Interrupt */
extern void isr3(void);     /* Breakpoint */
extern void isr4(void);     /* Overflow */
extern void isr5(void);     /* Bound Range Exceeded */
extern void isr6(void);     /* Invalid Opcode */
extern void isr7(void);     /* Device Not Available */
extern void isr8(void);     /* Double Fault */
extern void isr9(void);     /* Coprocessor Segment Overrun (legacy) */
extern void isr10(void);    /* Invalid TSS */
extern void isr11(void);    /* Segment Not Present */
extern void isr12(void);    /* Stack-Segment Fault */
extern void isr13(void);    /* General Protection Fault */
extern void isr14(void);    /* Page Fault */
extern void isr15(void);    /* Reserved */
extern void isr16(void);    /* x87 Floating-Point Exception */
extern void isr17(void);    /* Alignment Check */
extern void isr18(void);    /* Machine Check */
extern void isr19(void);    /* SIMD Floating-Point Exception */
extern void isr20(void);    /* Virtualization Exception */
extern void isr21(void);    /* Control Protection Exception */
extern void isr22(void);    /* Reserved */
extern void isr23(void);    /* Reserved */
extern void isr24(void);    /* Reserved */
extern void isr25(void);    /* Reserved */
extern void isr26(void);    /* Reserved */
extern void isr27(void);    /* Reserved */
extern void isr28(void);    /* Hypervisor Injection Exception */
extern void isr29(void);    /* VMM Communication Exception */
extern void isr30(void);    /* Security Exception */
extern void isr31(void);    /* Reserved */

/* IRQ (Interrupt Request) declarations - Hardware interrupts 0-15 */
/* Mapped to interrupts 32-47 to avoid conflict with CPU exceptions */
extern void irq0(void);     /* Programmable Interval Timer */
extern void irq1(void);     /* Keyboard */
extern void irq2(void);     /* Cascade (used internally by PICs) */
extern void irq3(void);     /* COM2 */
extern void irq4(void);     /* COM1 */
extern void irq5(void);     /* LPT2 */
extern void irq6(void);     /* Floppy Disk */
extern void irq7(void);     /* LPT1 / Spurious */
extern void irq8(void);     /* CMOS Real-Time Clock */
extern void irq9(void);     /* Free for peripherals / legacy SCSI / NIC */
extern void irq10(void);    /* Free for peripherals / SCSI / NIC */
extern void irq11(void);    /* Free for peripherals / SCSI / NIC */
extern void irq12(void);    /* PS/2 Mouse */
extern void irq13(void);    /* FPU / Coprocessor / Inter-processor */
extern void irq14(void);    /* Primary ATA Hard Disk */
extern void irq15(void);    /* Secondary ATA Hard Disk */

/* IRQ numbers (remapped to avoid CPU exception conflicts) */
#define IRQ0    32
#define IRQ1    33
#define IRQ2    34
#define IRQ3    35
#define IRQ4    36
#define IRQ5    37
#define IRQ6    38
#define IRQ7    39
#define IRQ8    40
#define IRQ9    41
#define IRQ10   42
#define IRQ11   43
#define IRQ12   44
#define IRQ13   45
#define IRQ14   46
#define IRQ15   47

#endif /* IDT_H */
