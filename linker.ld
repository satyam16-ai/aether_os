/* Aether OS Linker Script for Multiboot kernel */
ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
SECTIONS
{
  . = 1M; /* Load at 1MB for GRUB (identity mapped initially) */

  .multiboot : { *(.multiboot) }
  .text : ALIGN(4K) { *(.text*) }
  .rodata : ALIGN(4K) { *(.rodata*) }
  .data : ALIGN(4K) { *(.data*) }
  .bss : ALIGN(4K) {
    __bss_start = .;
    *(.bss*) *(COMMON)
    __bss_end = .;
  }

  /DISCARD/ : { *(.comment) *(.eh_frame) }
}
