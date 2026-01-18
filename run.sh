#!/bin/bash
#
# AJOS Operating System Build and Test Script
# For macOS development environment
#
# Usage:
#   ./run.sh          - Build and run in QEMU (default)
#   ./run.sh run      - Build and run in QEMU
#   ./run.sh build    - Build the ISO only
#   ./run.sh vbox     - Build and run in VirtualBox
#   ./run.sh vbox-del - Delete the VirtualBox VM
#   ./run.sh clean    - Remove build artifacts
#   ./run.sh deps     - Install dependencies only
#

set -e  # Exit on error
set -o pipefail  # Exit on pipe failure

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored messages
info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

warn() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Check if a command exists
command_exists() {
    command -v "$1" &> /dev/null
}

# Check if a brew package is installed
brew_installed() {
    brew list "$1" &> /dev/null 2>&1
}

# Install Homebrew if not present
install_homebrew() {
    if ! command_exists brew; then
        info "Installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        success "Homebrew installed"
    else
        info "Homebrew already installed, skipping..."
    fi
}

# Install a package via Homebrew (idempotent)
install_brew_package() {
    local package="$1"
    local check_cmd="${2:-$1}"  # Command to check, defaults to package name

    if command_exists "$check_cmd"; then
        info "$package already installed, skipping..."
        return 0
    fi

    if brew_installed "$package"; then
        info "$package already installed (via brew), skipping..."
        return 0
    fi

    info "Installing $package..."
    brew install "$package"
    success "$package installed"
}

# Install i686-elf cross-compiler toolchain
install_cross_compiler() {
    if command_exists i686-elf-gcc; then
        info "i686-elf-gcc cross-compiler already installed, skipping..."
        return 0
    fi

    info "Installing i686-elf cross-compiler toolchain..."

    # Try different tap names that might have the toolchain
    local taps=(
        "nativeos/i386-elf-toolchain"
        "messense/macos-cross-toolchains"
    )

    local installed=false

    # First try the nativeos tap
    if ! brew tap | grep -q "nativeos/i386-elf-toolchain"; then
        info "Adding tap nativeos/i386-elf-toolchain..."
        if brew tap nativeos/i386-elf-toolchain 2>/dev/null; then
            info "Tap added successfully"
        else
            warn "Could not add nativeos tap, trying alternatives..."
        fi
    fi

    # Try to install i686-elf-gcc
    if brew install i686-elf-gcc 2>/dev/null; then
        installed=true
        success "i686-elf-gcc installed via nativeos tap"
    fi

    # If that didn't work, try i386-elf-gcc
    if [ "$installed" = false ]; then
        if brew install i386-elf-gcc 2>/dev/null; then
            installed=true
            success "i386-elf-gcc installed"
            # Create symlink if needed
            if ! command_exists i686-elf-gcc && command_exists i386-elf-gcc; then
                warn "Creating symlink from i386-elf-gcc to i686-elf-gcc"
                local gcc_path=$(which i386-elf-gcc)
                local bin_dir=$(dirname "$gcc_path")
                ln -sf "$gcc_path" "$bin_dir/i686-elf-gcc" 2>/dev/null || true
            fi
        fi
    fi

    # If still not installed, try building from source
    if [ "$installed" = false ]; then
        warn "Pre-built cross-compiler not available"
        info "Attempting to install via homebrew-core x86_64-elf-gcc..."
        if brew install x86_64-elf-gcc 2>/dev/null; then
            installed=true
            warn "Installed x86_64-elf-gcc - you may need to adjust your Makefile for 64-bit"
        fi
    fi

    if [ "$installed" = false ]; then
        error "Could not install cross-compiler. Please install manually:

Option 1: Try installing from source
  brew tap nativeos/i386-elf-toolchain
  brew install i686-elf-gcc

Option 2: Build cross-compiler manually
  See: https://wiki.osdev.org/GCC_Cross-Compiler"
    fi
}

# Install all dependencies
install_dependencies() {
    info "Checking and installing dependencies..."
    echo ""

    # Ensure Homebrew is installed
    install_homebrew

    # Install required packages
    install_brew_package "nasm" "nasm"
    install_brew_package "qemu" "qemu-system-i386"
    install_brew_package "xorriso" "xorriso"

    # Install GRUB for grub-mkrescue (creates bootable ISOs)
    # The i686-elf-grub package installs as i686-elf-grub-mkrescue
    if ! command_exists i686-elf-grub-mkrescue && ! command_exists grub-mkrescue; then
        info "Installing i686-elf-grub (for grub-mkrescue)..."
        brew install i686-elf-grub
        success "i686-elf-grub installed"
    else
        info "grub-mkrescue already installed, skipping..."
    fi

    # Install cross-compiler
    install_cross_compiler

    echo ""
    success "All dependencies installed!"
}

# Build the OS
build() {
    info "Building AJOS..."

    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        error "Makefile not found in current directory. Are you in the AJOS source directory?"
    fi

    # Run make
    make

    # Check if ISO was created
    if [ -f "ajos.iso" ]; then
        success "Build complete: ajos.iso created"
    else
        error "Build failed: ajos.iso not found"
    fi
}

# Run in QEMU
run_qemu() {
    info "Starting QEMU..."

    if [ ! -f "ajos.iso" ]; then
        error "ajos.iso not found. Run './run.sh build' first."
    fi

    if ! command_exists qemu-system-i386; then
        error "qemu-system-i386 not found. Run './run.sh deps' to install."
    fi

    info "Launching AJOS in QEMU (close QEMU window to exit)..."
    qemu-system-i386 -cdrom ajos.iso
}

# Clean build artifacts
clean() {
    info "Cleaning build artifacts..."

    if [ -f "Makefile" ]; then
        # Try make clean if available
        if make clean 2>/dev/null; then
            success "Cleaned via 'make clean'"
        else
            # Manual cleanup of common artifacts
            rm -f *.o *.bin *.iso *.elf 2>/dev/null || true
            rm -rf isodir build obj 2>/dev/null || true
            success "Cleaned build artifacts"
        fi
    else
        # Manual cleanup
        rm -f *.o *.bin *.iso *.elf 2>/dev/null || true
        rm -rf isodir build obj 2>/dev/null || true
        success "Cleaned build artifacts"
    fi
}

# VirtualBox VM configuration
VBOX_VM_NAME="AJOS"
VBOX_MEMORY=128
VBOX_VRAM=16

# Install VirtualBox if not present (idempotent)
install_virtualbox() {
    if command_exists VBoxManage; then
        info "VirtualBox already installed, skipping..."
        return 0
    fi

    info "Installing VirtualBox..."
    brew install --cask virtualbox

    # Check if it installed successfully
    if command_exists VBoxManage; then
        success "VirtualBox installed"
    else
        warn "VirtualBox installed but VBoxManage not in PATH yet."
        warn "You may need to:"
        warn "  1. Allow the kernel extension in System Settings → Privacy & Security"
        warn "  2. Restart your terminal or computer"
        warn "Then run './run.sh vbox' again."
        exit 1
    fi
}

# Check if VirtualBox VM exists
vbox_vm_exists() {
    VBoxManage list vms 2>/dev/null | grep -q "\"$VBOX_VM_NAME\""
}

# Create VirtualBox VM if it doesn't exist
create_vbox_vm() {
    if vbox_vm_exists; then
        info "VM '$VBOX_VM_NAME' already exists"
        return 0
    fi

    info "Creating VirtualBox VM '$VBOX_VM_NAME'..."

    # Create the VM (Other = generic OS, 32-bit)
    VBoxManage createvm --name "$VBOX_VM_NAME" --ostype "Other" --register

    # Configure VM settings
    VBoxManage modifyvm "$VBOX_VM_NAME" \
        --memory $VBOX_MEMORY \
        --vram $VBOX_VRAM \
        --ioapic on \
        --boot1 dvd \
        --boot2 none \
        --boot3 none \
        --boot4 none \
        --audio-driver none

    # Add IDE controller for CD/DVD
    VBoxManage storagectl "$VBOX_VM_NAME" \
        --name "IDE Controller" \
        --add ide

    success "VirtualBox VM created"
}

# Attach ISO to VirtualBox VM
attach_iso_to_vbox() {
    local iso_path="$(pwd)/ajos.iso"

    if [ ! -f "$iso_path" ]; then
        error "ajos.iso not found. Run './run.sh build' first."
    fi

    info "Attaching ISO to VM..."

    # Detach any existing medium first (ignore errors if nothing attached)
    VBoxManage storageattach "$VBOX_VM_NAME" \
        --storagectl "IDE Controller" \
        --port 0 \
        --device 0 \
        --type dvddrive \
        --medium none 2>/dev/null || true

    # Attach the new ISO
    VBoxManage storageattach "$VBOX_VM_NAME" \
        --storagectl "IDE Controller" \
        --port 0 \
        --device 0 \
        --type dvddrive \
        --medium "$iso_path"

    success "ISO attached"
}

# Stop VirtualBox VM if running
stop_vbox_vm() {
    if VBoxManage list runningvms 2>/dev/null | grep -q "\"$VBOX_VM_NAME\""; then
        info "Stopping running VM..."
        VBoxManage controlvm "$VBOX_VM_NAME" poweroff 2>/dev/null || true
        sleep 2
    fi
}

# Check if running on ARM Mac
is_arm_mac() {
    [[ "$(uname -m)" == "arm64" ]]
}

# Run in VirtualBox
run_vbox() {
    # Check for ARM Mac - VirtualBox can't emulate x86 on ARM
    if is_arm_mac; then
        error "VirtualBox cannot run x86 VMs on ARM Macs (M1/M2/M3).

VirtualBox only virtualizes (same architecture), it cannot emulate.
QEMU can emulate x86 on ARM, so use:

    ./run.sh        # Uses QEMU (works on ARM Macs)

If you need VirtualBox specifically, you would need an Intel Mac."
    fi

    # Install VirtualBox if needed
    install_virtualbox

    if [ ! -f "ajos.iso" ]; then
        error "ajos.iso not found. Run './run.sh build' first."
    fi

    # Stop VM if already running
    stop_vbox_vm

    # Create VM if it doesn't exist
    create_vbox_vm

    # Attach the ISO
    attach_iso_to_vbox

    # Start the VM with GUI
    info "Starting VirtualBox VM '$VBOX_VM_NAME'..."
    VBoxManage startvm "$VBOX_VM_NAME" --type gui

    success "VM started! Close the VirtualBox window when done."
}

# Delete VirtualBox VM
delete_vbox_vm() {
    if ! command_exists VBoxManage; then
        info "VirtualBox not installed, nothing to delete"
        return 0
    fi

    if ! vbox_vm_exists; then
        info "VM '$VBOX_VM_NAME' does not exist"
        return 0
    fi

    # Stop if running
    stop_vbox_vm

    info "Deleting VM '$VBOX_VM_NAME'..."
    VBoxManage unregistervm "$VBOX_VM_NAME" --delete
    success "VM deleted"
}

# Print usage
usage() {
    echo "AJOS Build Script"
    echo ""
    echo "Usage: $0 [command]"
    echo ""
    echo "Commands:"
    echo "  run       Build and run in QEMU (default)"
    echo "  build     Build the ISO only"
    echo "  vbox      Build and run in VirtualBox"
    echo "  vbox-del  Delete the VirtualBox VM"
    echo "  clean     Remove build artifacts"
    echo "  deps      Install dependencies only"
    echo "  help      Show this help message"
    echo ""
}

# Main entry point
main() {
    local command="${1:-run}"

    case "$command" in
        run)
            install_dependencies
            build
            run_qemu
            ;;
        build)
            install_dependencies
            build
            ;;
        vbox)
            install_dependencies
            build
            run_vbox
            ;;
        vbox-del)
            delete_vbox_vm
            ;;
        clean)
            clean
            ;;
        deps)
            install_dependencies
            ;;
        help|--help|-h)
            usage
            ;;
        *)
            error "Unknown command: $command"
            usage
            exit 1
            ;;
    esac
}

# Run main with all arguments
main "$@"
