# AJOS - A Hobby Operating System

A 32-bit operating system built from scratch for learning purposes.

## Features

- Boots via GRUB (Multiboot specification)
- 32-bit protected mode (i686 architecture)
- GDT and IDT setup
- Hardware interrupt handling (PIC)
- PS/2 keyboard driver
- VGA text mode with hardware cursor
- Interactive shell with commands

## Shell Commands

All commands use the `aj` prefix:

| Command | Description |
|---------|-------------|
| `aj help` | List available commands |
| `aj clear` | Clear the screen |
| `aj echo [text]` | Print text |
| `aj version` | Show AJOS version |
| `aj reboot` | Reboot the system |
| `aj halt` | Halt the CPU |

## Building & Running

### Prerequisites (macOS)

The build script automatically installs dependencies:
- nasm (assembler)
- qemu (emulator)
- xorriso (ISO creation)
- i686-elf-gcc (cross-compiler)
- grub (bootloader tools)

### Quick Start

```bash
# Build and run in QEMU
./run.sh

# Build only
./run.sh build

# Clean build artifacts
./run.sh clean
```

## Project Structure

```
AJOS/
├── boot/
│   ├── boot.asm          # Multiboot entry point
│   ├── gdt_flush.asm     # GDT loading
│   └── interrupts.asm    # ISR/IRQ stubs
├── kernel/
│   ├── kernel.c          # Main kernel
│   ├── vga.c             # VGA text driver
│   ├── gdt.c             # Global Descriptor Table
│   ├── idt.c             # Interrupt Descriptor Table
│   ├── pic.c             # PIC controller
│   ├── keyboard.c        # PS/2 keyboard driver
│   ├── shell.c           # Interactive shell
│   └── string.c          # String utilities
├── include/              # Header files
├── Makefile
├── linker.ld             # Linker script
└── run.sh                # Build & run script
```

## Architecture

```
Boot Process:
BIOS → GRUB → boot.asm → kernel_main()
                ↓
         GDT → IDT → PIC → Keyboard → Shell
```

## Roadmap

- [x] v0.1 - Boot and print to screen
- [x] v0.2 - GDT, IDT, keyboard, shell
- [x] v0.3 - Hardware cursor, backspace fix
- [ ] v0.4 - VESA graphics mode
- [ ] v0.5 - Mouse driver
- [ ] v0.6 - Window manager & GUI

## License

MIT License - Feel free to use and learn from this code.

## Acknowledgments

Built with guidance from:
- [OSDev Wiki](https://wiki.osdev.org/)
- [James Molloy's Kernel Tutorial](http://www.jamesmolloy.co.uk/tutorial_html/)
