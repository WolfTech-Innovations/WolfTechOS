# WolfTech OS Enterprise Edition - Makefile

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -O2
LDFLAGS = -m elf_i386 -T kernel/linker.ld
ASFLAGS = -f elf32

BUILD_DIR = build
BOOT_DIR = boot
KERNEL_DIR = kernel
SERVER_DIR = server
USERSPACE_DIR = userspace

# Source files
BOOT_SRC = $(BOOT_DIR)/boot.asm
KERNEL_ASM = $(KERNEL_DIR)/entry.asm
KERNEL_C = $(KERNEL_DIR)/kernel.c
SERVER_C = $(SERVER_DIR)/server.c
DESKTOP_C = $(USERSPACE_DIR)/desktop.c

# Object files
KERNEL_ASM_OBJ = $(BUILD_DIR)/entry.o
KERNEL_C_OBJ = $(BUILD_DIR)/kernel.o
SERVER_OBJ = $(BUILD_DIR)/server.o
DESKTOP_OBJ = $(BUILD_DIR)/desktop.o

# Output files
BOOTLOADER = $(BUILD_DIR)/boot.bin
KERNEL = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/wolftech-os.img

.PHONY: all clean run run-fullscreen info

all: $(OS_IMAGE) info

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build bootloader
$(BOOTLOADER): $(BOOT_SRC) | $(BUILD_DIR)
	@echo "Building WolfTech bootloader..."
	$(AS) -f bin $(BOOT_SRC) -o $(BOOTLOADER)

# Build kernel ASM
$(KERNEL_ASM_OBJ): $(KERNEL_ASM) | $(BUILD_DIR)
	@echo "Assembling kernel entry..."
	$(AS) $(ASFLAGS) $(KERNEL_ASM) -o $(KERNEL_ASM_OBJ)

# Build kernel C
$(KERNEL_C_OBJ): $(KERNEL_C) | $(BUILD_DIR)
	@echo "Compiling kernel..."
	$(CC) $(CFLAGS) -c $(KERNEL_C) -o $(KERNEL_C_OBJ)

# Build server
$(SERVER_OBJ): $(SERVER_C) | $(BUILD_DIR)
	@echo "Compiling enterprise server..."
	$(CC) $(CFLAGS) -c $(SERVER_C) -o $(SERVER_OBJ)

# Build desktop
$(DESKTOP_OBJ): $(DESKTOP_C) | $(BUILD_DIR)
	@echo "Compiling desktop environment..."
	$(CC) $(CFLAGS) -c $(DESKTOP_C) -o $(DESKTOP_OBJ)

# Link kernel
$(KERNEL): $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(SERVER_OBJ) $(DESKTOP_OBJ) | $(BUILD_DIR)
	@echo "Linking WolfTech OS kernel..."
	$(LD) $(LDFLAGS) -o $(KERNEL) $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(SERVER_OBJ) $(DESKTOP_OBJ)

# Create OS image
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL)
	@echo "Creating bootable image..."
	dd if=/dev/zero of=$(OS_IMAGE) bs=512 count=2880 2>/dev/null
	dd if=$(BOOTLOADER) of=$(OS_IMAGE) conv=notrunc 2>/dev/null
	dd if=$(KERNEL) of=$(OS_IMAGE) seek=1 conv=notrunc 2>/dev/null
	@echo "WolfTech OS image created: $(OS_IMAGE)"

info:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════════════╗"
	@echo "║         WOLFTECH OS ENTERPRISE EDITION - BUILD COMPLETE        ║"
	@echo "╠════════════════════════════════════════════════════════════════╣"
	@echo "║                                                                ║"
	@echo "║  Features:                                                     ║"
	@echo "║    ✓ Custom bootloader with animated boot screen              ║"
	@echo "║    ✓ ELF binary loader (Linux compatibility)                  ║"
	@echo "║    ✓ Linux syscall translation layer                          ║"
	@echo "║    ✓ Employee database system                                 ║"
	@echo "║    ✓ SSO (Single Sign-On) server                              ║"
	@echo "║    ✓ Web dashboard (HTTP server)                              ║"
	@echo "║    ✓ Wallpaper engine with animations                         ║"
	@echo "║    ✓ Driver framework                                         ║"
	@echo "║    ✓ Enterprise desktop environment                           ║"
	@echo "║                                                                ║"
	@echo "║  Image: build/wolftech-os.img                                 ║"
	@echo "║  Size:  1.44 MB (floppy disk image)                           ║"
	@echo "║                                                                ║"
	@echo "║  Run with: make run                                           ║"
	@echo "║                                                                ║"
	@echo "╚════════════════════════════════════════════════════════════════╝"
	@echo ""

# Run in QEMU
run: $(OS_IMAGE)
	@echo "Starting WolfTech OS in QEMU..."
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 128M -vga std

run-fullscreen: $(OS_IMAGE)
	@echo "Starting WolfTech OS in QEMU (fullscreen)..."
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 128M -vga std -full-screen

# Run with debugging
debug: $(OS_IMAGE)
	@echo "Starting WolfTech OS in debug mode..."
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE) -m 128M -vga std -s -S

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	@echo "Clean complete."
