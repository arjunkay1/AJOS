; AJOS Interrupt Service Routine Stubs
; These stubs save CPU state, call C handlers, restore state, and return

bits 32

; External C handlers
extern isr_handler
extern irq_handler

; ============================================================================
; ISR Stubs (CPU Exceptions 0-31)
; ============================================================================

; Macro for ISR with no error code (pushes dummy error code)
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0        ; Dummy error code
    push dword %1       ; Interrupt number
    jmp isr_common_stub
%endmacro

; Macro for ISR with error code (CPU pushes error code automatically)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    push dword %1       ; Interrupt number (error code already on stack)
    jmp isr_common_stub
%endmacro

; CPU Exceptions
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Into Detected Overflow
ISR_NOERRCODE 5     ; Out of Bounds
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; No Coprocessor
ISR_ERRCODE   8     ; Double Fault (has error code)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Bad TSS (has error code)
ISR_ERRCODE   11    ; Segment Not Present (has error code)
ISR_ERRCODE   12    ; Stack Fault (has error code)
ISR_ERRCODE   13    ; General Protection Fault (has error code)
ISR_ERRCODE   14    ; Page Fault (has error code)
ISR_NOERRCODE 15    ; Unknown Interrupt
ISR_NOERRCODE 16    ; Coprocessor Fault
ISR_ERRCODE   17    ; Alignment Check (has error code)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; Reserved
ISR_NOERRCODE 20    ; Reserved
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception (has error code)
ISR_NOERRCODE 31    ; Reserved

; Common ISR stub - saves registers and calls C handler
isr_common_stub:
    ; Save all general purpose registers
    pusha

    ; Save data segment
    mov ax, ds
    push eax

    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push pointer to register structure as argument
    push esp

    ; Call C handler: isr_handler(registers_t *regs)
    call isr_handler

    ; Clean up pushed argument
    add esp, 4

    ; Restore data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore all general purpose registers
    popa

    ; Clean up pushed error code and interrupt number
    add esp, 8

    ; Return from interrupt
    iret

; ============================================================================
; IRQ Stubs (Hardware Interrupts 0-15 -> Interrupts 32-47)
; ============================================================================

; Macro for IRQ stub
%macro IRQ 2
global irq%1
irq%1:
    push dword 0        ; Dummy error code
    push dword %2       ; Interrupt number (32 + IRQ number)
    jmp irq_common_stub
%endmacro

; Hardware IRQs
IRQ 0, 32       ; Timer (PIT)
IRQ 1, 33       ; Keyboard
IRQ 2, 34       ; Cascade (never raised)
IRQ 3, 35       ; COM2
IRQ 4, 36       ; COM1
IRQ 5, 37       ; LPT2
IRQ 6, 38       ; Floppy Disk
IRQ 7, 39       ; LPT1 / Spurious
IRQ 8, 40       ; CMOS Real Time Clock
IRQ 9, 41       ; Free / ACPI
IRQ 10, 42      ; Free
IRQ 11, 43      ; Free
IRQ 12, 44      ; PS/2 Mouse
IRQ 13, 45      ; FPU / Coprocessor
IRQ 14, 46      ; Primary ATA Hard Disk
IRQ 15, 47      ; Secondary ATA Hard Disk

; Common IRQ stub - saves registers and calls C handler
irq_common_stub:
    ; Save all general purpose registers
    pusha

    ; Save data segment
    mov ax, ds
    push eax

    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Push pointer to register structure as argument
    push esp

    ; Call C handler: irq_handler(registers_t *regs)
    call irq_handler

    ; Clean up pushed argument
    add esp, 4

    ; Restore data segment
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore all general purpose registers
    popa

    ; Clean up pushed error code and interrupt number
    add esp, 8

    ; Return from interrupt
    iret
