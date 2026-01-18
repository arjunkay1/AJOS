#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "shell.h"

/*
 * Kernel entry point
 * Called from boot.asm after setting up the initial environment
 */
void kernel_main(void) {
    /* Step 1: Initialize VGA text mode */
    vga_init();

    /* Step 2: Initialize Global Descriptor Table */
    gdt_init();

    /* Step 3: Initialize Interrupt Descriptor Table */
    idt_init();

    /* Step 4: Initialize and remap the PIC (Programmable Interrupt Controller) */
    pic_init();

    /* Step 5: Initialize keyboard driver */
    keyboard_init();

    /* Step 6: Enable interrupts */
    __asm__ volatile ("sti");

    /* Step 7: Print welcome message */
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_print("Welcome to AJOS - A Just Operating System\n");
    vga_print("==========================================\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_print("Type 'aj help' for a list of available commands.\n\n");

    /* Step 8: Run the shell (main loop - never returns) */
    shell_run();

    /* Should never reach here */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
