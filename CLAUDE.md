# AJOS - A Hobby Operating System

## What Is This Project?
AJOS is a 32-bit operating system built from scratch for learning purposes. The OS boots via GRUB, runs on x86 (i686) architecture, and is tested using QEMU on macOS.

## How It Works

### Boot Process
1. **BIOS** initializes hardware and loads GRUB from disk
2. **GRUB** reads our `grub.cfg`, loads `kernel.bin` into memory at 1MB
3. **boot.asm** receives control (via Multiboot), sets up the stack, calls `kernel_main`
4. **kernel.c** initializes VGA, clears screen, prints boot message
5. Later: GDT, IDT, keyboard driver, shell

### Memory Layout
```
0x00000000 - 0x000FFFFF  : Reserved (BIOS, VGA memory at 0xB8000)
0x00100000 - onwards     : Our kernel loads here (1MB mark)
```

### VGA Text Mode
- Memory-mapped at `0xB8000`
- 80 columns x 25 rows
- Each character = 2 bytes: [ASCII char][color attribute]
- Color: high nibble = background, low nibble = foreground

---

## Key Decisions & Why

### Why 32-bit (i686) instead of 64-bit?
- Simpler: no mandatory paging, simpler memory model
- Better tutorials and documentation available
- Can upgrade to 64-bit later once fundamentals are solid

### Why GRUB instead of custom bootloader?
- Bootloaders are complex: real mode → protected mode, A20 gate, disk I/O
- GRUB handles all this; we focus on actual OS development
- Multiboot spec is well-documented and widely used

### Why VGA text mode first?
- Available immediately after boot (no driver needed)
- Simple: just write bytes to 0xB8000
- Good enough for initial development and debugging

---

## Project Structure
```
ajos/
├── CLAUDE.md           # This file - project docs
├── run.sh              # Build & test script (idempotent)
├── Makefile            # Build rules
├── linker.ld           # Linker script (memory layout)
├── boot/
│   └── boot.asm        # Multiboot entry point
├── kernel/
│   ├── kernel.c        # Main kernel
│   └── vga.c           # VGA text driver
├── include/
│   └── vga.h           # Headers
└── iso/
    └── boot/
        └── grub/
            └── grub.cfg    # GRUB config
```

---

## Build & Run
```bash
./run.sh              # Install deps, build, run in QEMU (default)
./run.sh build        # Build only
./run.sh vbox         # Build and run in VirtualBox
./run.sh vbox-del     # Delete the VirtualBox VM
./run.sh clean        # Clean build artifacts
./run.sh deps         # Install dependencies only
```

### VirtualBox Support
The `vbox` command automatically:
1. Installs VirtualBox (if not installed)
2. Creates a VM named "AJOS" (if it doesn't exist)
3. Configures: 128MB RAM, boot from DVD, no audio
4. Attaches the ISO to IDE controller
5. Starts with GUI mode

**ARM Mac Limitation:** VirtualBox cannot run x86 VMs on M1/M2/M3 Macs. Use `./run.sh` (QEMU) instead - QEMU emulates x86, VirtualBox only virtualizes.

---

## Development Log

### Session 1 - Initial Setup
- Created project structure
- Chose 32-bit i686 target for simplicity
- Chose GRUB + Multiboot (handles boot complexity)
- Created idempotent run.sh for easy testing
- Goal: Boot and print "AJOS" to screen

**Files created:**
- `run.sh` - Idempotent script that installs deps (nasm, qemu, xorriso, grub, i686-elf-gcc), builds, and runs

### Session 2 - Interactive Shell (v0.2 → v0.3)
- Implemented GDT (Global Descriptor Table)
- Implemented IDT + PIC (Interrupt handling, remapped IRQs)
- Implemented PS/2 keyboard driver with scancode-to-ASCII
- Built interactive shell with `aj` command prefix
- Fixed backspace handling in VGA driver
- Added hardware cursor tracking (cursor follows typing)

**New files:**
- `include/gdt.h`, `kernel/gdt.c`, `boot/gdt_flush.asm` - GDT
- `include/idt.h`, `kernel/idt.c`, `boot/interrupts.asm` - IDT
- `include/pic.h`, `kernel/pic.c` - PIC controller
- `include/io.h` - Port I/O functions
- `include/keyboard.h`, `kernel/keyboard.c` - Keyboard driver
- `include/shell.h`, `kernel/shell.c` - Interactive shell
- `include/string.h`, `kernel/string.c` - String utilities
- `Makefile` - Builds boot.asm → .o, kernel/*.c → .o, links to kernel.bin, creates ISO
- `linker.ld` - Loads kernel at 1MB (0x100000), sections: .multiboot, .text, .rodata, .data, .bss
- `boot/boot.asm` - Multiboot header, 16KB stack, calls kernel_main
- `kernel/vga.c` - VGA text mode driver (0xB8000), 80x25, color support
- `kernel/kernel.c` - Main entry, prints "AJOS v0.1"
- `include/vga.h` - VGA function declarations
- `iso/boot/grub/grub.cfg` - GRUB menu entry for AJOS

---

## Phase Overview

### Phase 1: Build Infrastructure
- [x] CLAUDE.md documentation
- [x] run.sh idempotent script
- [x] Makefile
- [x] linker.ld

### Phase 2: Bootable Kernel
- [x] boot.asm (Multiboot header, stack setup)
- [x] vga.c (text output)
- [x] kernel.c (main entry)
- [x] GRUB config
- [x] Boot test

### Phase 3: OS Foundations
- [x] GDT (Global Descriptor Table)
- [x] IDT (Interrupt Descriptor Table)
- [x] PIC (remaps IRQs to avoid conflicts)
- [x] Keyboard driver (PS/2, scancode set 1)
- [x] Basic shell with commands
- [x] Hardware cursor tracking (v0.3)

### Available Shell Commands
All commands use the `aj` prefix:
- `aj help` - List available commands
- `aj clear` - Clear the screen
- `aj echo [text]` - Print text
- `aj version` - Show AJOS version
- `aj reboot` - Reboot the system
- `aj halt` - Halt the CPU

### Phase 4: Graphics & GUI (Next)
- [ ] VESA framebuffer (switch to graphics mode)
- [ ] Drawing primitives (pixels, lines, rectangles)
- [ ] Bitmap font rendering
- [ ] Mouse driver (PS/2)
- [ ] Window manager
- [ ] Basic widgets (buttons, text)
