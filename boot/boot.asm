; AJOS Bootloader - Multiboot compliant entry point
; NASM syntax, 32-bit protected mode

bits 32

; Multiboot header constants
MBALIGN         equ 1 << 0          ; Align loaded modules on page boundaries
MEMINFO         equ 1 << 1          ; Provide memory map
VIDEO_MODE      equ 1 << 2          ; Request video mode info (bit 2)
FLAGS           equ MBALIGN | MEMINFO | VIDEO_MODE  ; Multiboot flags
MAGIC           equ 0x1BADB002      ; Multiboot magic number
CHECKSUM        equ -(MAGIC + FLAGS) ; Checksum (magic + flags + checksum = 0)

; Video mode settings
MODE_TYPE       equ 0               ; Linear graphics mode (not text)
WIDTH           equ 800             ; Screen width
HEIGHT          equ 600             ; Screen height
DEPTH           equ 32              ; Bits per pixel

; Multiboot header section
section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
    dd 0                            ; header_addr (unused without AOUT_KLUDGE)
    dd 0                            ; load_addr (unused)
    dd 0                            ; load_end_addr (unused)
    dd 0                            ; bss_end_addr (unused)
    dd 0                            ; entry_addr (unused)
    dd MODE_TYPE                    ; mode_type: 0 = linear graphics
    dd WIDTH                        ; width: 800 pixels
    dd HEIGHT                       ; height: 600 pixels
    dd DEPTH                        ; depth: 32 bits per pixel

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

    ; Push multiboot info pointer (ebx) as argument to kernel_main
    ; GRUB passes: eax = magic number, ebx = multiboot info pointer
    push ebx

    ; Call the kernel main function
    call kernel_main

    ; Clean up stack (1 argument)
    add esp, 4

    ; If kernel_main returns, enter infinite halt loop
    cli                             ; Disable interrupts
.hang:
    hlt                             ; Halt the CPU
    jmp .hang                       ; Loop forever (in case of NMI)
