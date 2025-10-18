# Aether OS Build System
# --------------------------------------------------------
# Supports building a Multiboot (GRUB) friendly kernel + ISO.
# Provides fallback to host gcc if cross i686-elf toolchain not found.

CROSS ?= i686-elf-
ARCH  ?= i386

AS      := nasm
CC_X    := $(CROSS)gcc
LD_X    := $(CROSS)ld
OBJCOPY_X := $(CROSS)objcopy

CC_SYS  := gcc
LD_SYS  := ld
OBJCOPY_SYS := objcopy

# Detect cross tools
HAVE_CROSS_GCC := $(shell which $(CC_X) 2>/dev/null)

ifeq ($(strip $(HAVE_CROSS_GCC)),)
  $(warning Cross compiler $(CC_X) not found; falling back to system gcc. Install i686-elf-gcc for a clean freestanding build.)
  CC := $(CC_SYS)
  LD := $(LD_SYS)
  OBJCOPY := $(OBJCOPY_SYS)
  CFLAGS_ARCH := -m32
  LDFLAGS_ARCH := -m elf_i386
else
  CC := $(CC_X)
  LD := $(LD_X)
  OBJCOPY := $(OBJCOPY_X)
  CFLAGS_ARCH :=
  LDFLAGS_ARCH :=
endif

BUILD_DIR := build
KERNEL_DIR := $(BUILD_DIR)/kernel
ISO_DIR := $(BUILD_DIR)/isodir
GRUB_DIR := $(ISO_DIR)/boot/grub
BOOT_DIR := $(BUILD_DIR)/boot

KERNEL_ELF := $(KERNEL_DIR)/kernel.elf
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin
ISO_IMAGE := $(BUILD_DIR)/aether.iso

C_SOURCES := $(wildcard src/kernel/*.c)
ASM_SOURCES := $(wildcard src/kernel/*.asm)
OBJ := $(patsubst src/kernel/%.c,$(KERNEL_DIR)/%.o,$(C_SOURCES)) \
       $(patsubst src/kernel/%.asm,$(KERNEL_DIR)/%.o,$(ASM_SOURCES))

INCLUDE_DIR := include
CFLAGS := $(CFLAGS_ARCH) -ffreestanding -O2 -Wall -Wextra -Werror=implicit-function-declaration -nostdlib -nostdinc -fno-stack-protector -fno-pic -fno-omit-frame-pointer -g -I$(INCLUDE_DIR)
LDFLAGS := $(LDFLAGS_ARCH) -nostdlib -z max-page-size=0x1000 -T linker.ld
ASFLAGS := -f elf32

# External tools
GRUB_MKRESCUE := $(shell which grub-mkrescue 2>/dev/null)
XORRISO := $(shell which xorriso 2>/dev/null)
MTOOLS := $(shell which mtools 2>/dev/null)
QEMU ?= qemu-system-i386
QEMU_FLAGS ?= -boot d -cdrom $(ISO_IMAGE)

.PHONY: all kernel iso run clean distclean tree bootloader help

all: iso

# Kernel build
$(KERNEL_DIR):
	@mkdir -p $@

$(KERNEL_DIR)/%.o: src/kernel/%.c | $(KERNEL_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_DIR)/%.o: src/kernel/%.asm | $(KERNEL_DIR)
	$(AS) $(ASFLAGS) $< -o $@

$(KERNEL_ELF): $(OBJ) linker.ld | $(KERNEL_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

# Optional: raw binary (not required for multiboot / GRUB)
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# GRUB ISO
$(GRUB_DIR):
	@mkdir -p $@

$(GRUB_DIR)/grub.cfg: | $(GRUB_DIR)
	@printf '%s\n' \
	"set timeout=0" \
	"set default=0" \
	"menuentry 'Aether OS' {" \
	"  multiboot /boot/kernel.elf" \
	"  boot" \
	"}" > $@

iso: $(KERNEL_ELF) $(GRUB_DIR)/grub.cfg ## Build bootable ISO
	@if [ -z "$(GRUB_MKRESCUE)" ]; then \
	  echo "ERROR: grub-mkrescue not found. Install grub-pc-bin xorriso mtools."; exit 1; fi
	@mkdir -p $(ISO_DIR)/boot
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/kernel.elf
	$(GRUB_MKRESCUE) -o $(ISO_IMAGE) $(ISO_DIR)
	@echo "[+] ISO created at $(ISO_IMAGE)"

run: iso ## Build and run in QEMU (graphical)
	$(QEMU) $(QEMU_FLAGS)

run-headless: iso ## Run with serial output only (useful for logs)
	$(QEMU) -display none -serial stdio $(QEMU_FLAGS)

run-kernel: $(KERNEL_ELF) ## Directly run multiboot kernel without ISO (debug)
	$(QEMU) -kernel $(KERNEL_ELF) -serial stdio -no-reboot -d guest_errors

verify-iso: iso ## List contents of ISO to ensure kernel.elf is present
	@if [ -z "$(XORRISO)" ]; then echo "xorriso not installed"; exit 1; fi
	$(XORRISO) -indev $(ISO_IMAGE) -ls / | grep -E 'kernel\.elf|grub'

check-tools: ## Report status of required external tools
	@echo "Tool status:"; \
	for t in grub-mkrescue xorriso mtools nasm $(QEMU); do \
	  if which $$t >/dev/null 2>&1; then echo "  [OK] $$t -> `which $$t`"; else echo "  [MISS] $$t"; fi; \
	done

# Experimental: build raw bootloader binary (current custom boot sector) only
bootloader: src/bootloader/bootloader.asm | $(BOOT_DIR) ## Build only the custom boot sector
	$(AS) -f bin $< -o $(BOOT_DIR)/bootloader.bin

$(BOOT_DIR):
	@mkdir -p $@

clean: ## Remove intermediate build artifacts
	rm -rf $(BUILD_DIR)/kernel $(ISO_DIR) $(BOOT_DIR)

DistFiles = Makefile linker.ld readme.md src docs scripts

distclean: clean ## Remove all build artifacts including ISO
	rm -f $(ISO_IMAGE)

# Display a compact project tree (ignores build artifacts)
tree: ## Show trimmed project tree
	@echo "Project structure:" && find . -maxdepth 6 \( -path './build' -o -path './.git' \) -prune -o -print | sed 's|^./||'

print-%:
	@echo '$*=$($*)'

help: ## Show this help
	@grep -E '^[a-zA-Z_-]+:.*?##' Makefile | sed 's/:.*##/: /'

# (Add '## description' after targets to have them show here.)
