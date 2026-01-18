#ifndef GDT_H
#define GDT_H

#include <stdint.h>

/* Segment selector defines */
#define KERNEL_CODE_SEG 0x08
#define KERNEL_DATA_SEG 0x10

/* GDT entry structure (8 bytes) */
struct gdt_entry {
    uint16_t limit_low;     /* Lower 16 bits of limit */
    uint16_t base_low;      /* Lower 16 bits of base */
    uint8_t  base_middle;   /* Next 8 bits of base */
    uint8_t  access;        /* Access flags */
    uint8_t  granularity;   /* Granularity and upper 4 bits of limit */
    uint8_t  base_high;     /* Upper 8 bits of base */
} __attribute__((packed));

/* GDT pointer structure */
struct gdt_ptr {
    uint16_t limit;         /* Size of GDT - 1 */
    uint32_t base;          /* Base address of GDT */
} __attribute__((packed));

/* Function declarations */
void gdt_init(void);
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);

#endif /* GDT_H */
