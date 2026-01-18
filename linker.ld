/* AJOS Linker Script */
/* i686 32-bit kernel */

ENTRY(_start)

SECTIONS
{
    /* Load kernel at 1MB */
    . = 0x100000;

    /* Multiboot header must come first */
    .multiboot ALIGN(4K) :
    {
        *(.multiboot)
    }

    /* Code section */
    .text ALIGN(4K) :
    {
        __text_start = .;
        *(.text)
        __text_end = .;
    }

    /* Read-only data */
    .rodata ALIGN(4K) :
    {
        __rodata_start = .;
        *(.rodata)
        *(.rodata.*)
        __rodata_end = .;
    }

    /* Initialized data */
    .data ALIGN(4K) :
    {
        __data_start = .;
        *(.data)
        __data_end = .;
    }

    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss)
        __bss_end = .;
    }

    /* End of kernel */
    __kernel_end = .;
}
