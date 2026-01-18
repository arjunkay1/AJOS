#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "keyboard.h"
#include "shell.h"
#include "graphics.h"
#include "desktop.h"

/*
 * Kernel entry point
 * Called from boot.asm after setting up the initial environment
 *
 * @param multiboot_info Pointer to multiboot info structure passed by GRUB
 */
void kernel_main(void* multiboot_info) {
    /* Step 1: Initialize graphics from multiboot info */
    graphics_init(multiboot_info);

    /* Step 2: Initialize VGA text mode (fallback if no graphics) */
    vga_init();

    /* Step 3: Initialize Global Descriptor Table */
    gdt_init();

    /* Step 4: Initialize Interrupt Descriptor Table */
    idt_init();

    /* Step 5: Initialize and remap the PIC (Programmable Interrupt Controller) */
    pic_init();

    /* Step 6: Initialize keyboard driver */
    keyboard_init();

    /* Step 7: Enable interrupts */
    __asm__ volatile ("sti");

    /* Step 8: Check for graphics mode and run appropriate interface */
    if (graphics_is_available()) {
        /* Graphics mode available - run desktop environment */
        desktop_init();
        desktop_run();  /* Never returns */
    } else {
        /* Fallback to text mode shell */
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_print("Welcome to AJOS - A Just Operating System\n");
        vga_print("==========================================\n\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        vga_print("[Text Mode: VGA 80x25]\n\n");
        vga_print("Type 'aj help' for a list of available commands.\n\n");

        /* Run the shell (main loop - never returns) */
        shell_run();
    }

    /* Should never reach here */
    while (1) {
        __asm__ volatile ("hlt");
    }
}
