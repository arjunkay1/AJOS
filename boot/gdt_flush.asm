; GDT Flush - Load the Global Descriptor Table
; NASM syntax, 32-bit protected mode

bits 32

section .text
global gdt_flush

; gdt_flush - Load the GDT and reload segment registers
; Called from C: void gdt_flush(uint32_t gdt_ptr)
; Argument: pointer to GDT descriptor structure (on stack)
gdt_flush:
    ; Get the GDT pointer from the stack
    mov eax, [esp + 4]      ; First argument is at esp+4 (after return address)

    ; Load the GDT
    lgdt [eax]

    ; Reload CS by performing a far jump
    ; This loads the new code segment selector (0x08 = kernel code segment)
    jmp 0x08:.reload_segments

.reload_segments:
    ; Reload all data segment registers with kernel data segment (0x10)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret
