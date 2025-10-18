; Multiboot v1 header - simpler and more compatible
; Must appear in the first 8KiB of the file & be 4-byte aligned.

SECTION .multiboot
ALIGN 4
MB_MAGIC    equ 0x1BADB002
MB_FLAGS    equ 0x00000000 ; no extra sections requested yet
MB_CHECKSUM equ -(MB_MAGIC + MB_FLAGS)

dd MB_MAGIC
dd MB_FLAGS
dd MB_CHECKSUM
