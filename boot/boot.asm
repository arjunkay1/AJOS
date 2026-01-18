; AJOS Bootloader - Multiboot compliant entry point
; NASM syntax, 32-bit protected mode

bits 32

; Multiboot header constants
MBALIGN     equ 1 << 0              ; Align loaded modules on page boundaries
MEMINFO     equ 1 << 1              ; Provide memory map
FLAGS       equ MBALIGN | MEMINFO   ; Multiboot flags
MAGIC       equ 0x1BADB002          ; Multiboot magic number
CHECKSUM    equ -(MAGIC + FLAGS)    ; Checksum (magic + flags + checksum = 0)

; Multiboot header section
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; Stack section - 16KB aligned to 16 bytes
section .bss
align 16
stack_bottom:
    resb 16384                      ; 16 KB stack
stack_top:

; Text section - entry point
section .text
global _start
extern kernel_main

_start:
    ; Set up the stack pointer
    mov esp, stack_top

    ; Call the kernel main function
    call kernel_main

    ; If kernel_main returns, enter infinite halt loop
    cli                             ; Disable interrupts
.hang:
    hlt                             ; Halt the CPU
    jmp .hang                       ; Loop forever (in case of NMI)
