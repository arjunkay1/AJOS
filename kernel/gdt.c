#include "../include/gdt.h"

/* External assembly function to load GDT */
extern void gdt_flush(uint32_t gdt_ptr);

/* GDT with 3 entries: null, kernel code, kernel data */
static struct gdt_entry gdt[3];
static struct gdt_ptr gp;

/*
 * gdt_set_gate - Set up a GDT entry
 * @num: Index of the GDT entry
 * @base: Base address of the segment
 * @limit: Limit of the segment
 * @access: Access flags
 * @granularity: Granularity flags
 */
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    /* Set base address */
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    /* Set limit */
    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    /* Set granularity and access flags */
    gdt[num].granularity |= (granularity & 0xF0);
    gdt[num].access = access;
}

/*
 * gdt_init - Initialize the Global Descriptor Table
 *
 * Sets up 3 GDT entries:
 * - Entry 0: Null descriptor (required by CPU)
 * - Entry 1: Kernel code segment (0x08)
 * - Entry 2: Kernel data segment (0x10)
 */
void gdt_init(void) {
    /* Set up GDT pointer */
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base = (uint32_t)&gdt;

    /* Entry 0: Null descriptor - required by CPU */
    gdt_set_gate(0, 0, 0, 0, 0);

    /* Entry 1: Kernel code segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with 4KB granularity)
     * Access: 0x9A
     *   - Present (1)
     *   - Ring 0 (00)
     *   - Code/Data segment (1)
     *   - Executable (1)
     *   - Direction/Conforming (0)
     *   - Readable (1)
     *   - Accessed (0)
     * Granularity: 0xCF
     *   - 4KB blocks (1)
     *   - 32-bit protected mode (1)
     *   - 64-bit segment (0)
     *   - AVL (0)
     *   - Limit bits 19:16 (F)
     */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    /* Entry 2: Kernel data segment
     * Base: 0x00000000
     * Limit: 0xFFFFFFFF (4GB with 4KB granularity)
     * Access: 0x92
     *   - Present (1)
     *   - Ring 0 (00)
     *   - Code/Data segment (1)
     *   - Data segment (0)
     *   - Direction (0)
     *   - Writable (1)
     *   - Accessed (0)
     * Granularity: 0xCF (same as code segment)
     */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    /* Load the GDT */
    gdt_flush((uint32_t)&gp);
}
