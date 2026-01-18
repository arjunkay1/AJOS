# AJOS - A Hobby Operating System

A 32-bit operating system built from scratch for learning purposes.

## Features

### v1.0.0 - Graphical Desktop Environment
- **VESA Graphics Mode** - 800x600 resolution, 32-bit color
- **Window Manager** - Draggable windows with titlebar and close button
- **PS/2 Mouse Support** - Full cursor movement and click detection
- **Taskbar** - AJOS start button that opens new terminal windows
- **Terminal Emulator** - Command-line interface in a window
- **Double Buffering** - Flicker-free rendering

### Core OS Features
- Boots via GRUB (Multiboot specification)
- 32-bit protected mode (i686 architecture)
- GDT and IDT setup
- Hardware interrupt handling (PIC)
- PS/2 keyboard driver
- VGA text mode fallback

## Shell Commands

All commands use the `aj` prefix:

| Command | Description |
|---------|-------------|
| `aj help` | List available commands |
| `aj clear` | Clear the terminal |
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
│   ├── mouse.c           # PS/2 mouse driver
│   ├── graphics.c        # VESA framebuffer
│   ├── draw.c            # Drawing primitives
│   ├── font.c            # Bitmap font
│   ├── window.c          # Window manager
│   ├── terminal.c        # Terminal emulator
│   ├── taskbar.c         # Desktop taskbar
│   ├── desktop.c         # Desktop environment
│   ├── shell.c           # Text-mode shell
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
         GDT → IDT → PIC → Keyboard → Mouse
                ↓
         Graphics Mode Available?
              ↓           ↓
           Yes           No
              ↓           ↓
         Desktop      Text Shell
```

## Screenshots

When running in graphics mode, AJOS displays:
- Teal desktop background
- Terminal window with command prompt
- Gray taskbar at bottom with AJOS button
- Mouse cursor

## Roadmap

- [x] v0.1 - Boot and print to screen
- [x] v0.2 - GDT, IDT, keyboard, shell
- [x] v0.3 - Hardware cursor, backspace fix
- [x] v1.0.0 - Graphics mode, window manager, mouse, desktop

## License

MIT License - Feel free to use and learn from this code.

## Acknowledgments

Built with guidance from:
- [OSDev Wiki](https://wiki.osdev.org/)
- [James Molloy's Kernel Tutorial](http://www.jamesmolloy.co.uk/tutorial_html/)
