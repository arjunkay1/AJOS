# AJOS Build System
# 32-bit i686 Operating System

# Toolchain
AS = nasm
CC = i686-elf-gcc
LD = i686-elf-ld
GRUB_MKRESCUE = i686-elf-grub-mkrescue

# Directories
BUILD_DIR = build
ISO_DIR = iso
BOOT_DIR = boot
KERNEL_DIR = kernel

# Flags
ASFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -Wall -Wextra -Iinclude -c
LDFLAGS = -T linker.ld -nostdlib

# Source files
BOOT_SRC = $(wildcard $(BOOT_DIR)/*.asm)
KERNEL_SRC = $(wildcard $(KERNEL_DIR)/*.c)

# Object files
BOOT_OBJ = $(patsubst $(BOOT_DIR)/%.asm,$(BUILD_DIR)/%.o,$(BOOT_SRC))
KERNEL_OBJ = $(patsubst $(KERNEL_DIR)/%.c,$(BUILD_DIR)/%.o,$(KERNEL_SRC))
ALL_OBJ = $(BOOT_OBJ) $(KERNEL_OBJ)

# Output
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
ISO_FILE = ajos.iso

# Default target
all: $(ISO_FILE)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Assemble boot files
$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.asm | $(BUILD_DIR)
	$(AS) $(ASFLAGS) $< -o $@

# Compile kernel C files
$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Link kernel binary
$(KERNEL_BIN): $(ALL_OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

kernel.bin: $(KERNEL_BIN)

# Create bootable ISO
$(ISO_FILE): $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/kernel.bin
	echo 'menuentry "AJOS" {' > $(ISO_DIR)/boot/grub/grub.cfg
	echo '    multiboot /boot/kernel.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	$(GRUB_MKRESCUE) -o $(ISO_FILE) $(ISO_DIR)

iso: $(ISO_FILE)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(ISO_DIR) $(ISO_FILE)

.PHONY: all clean iso kernel.bin
