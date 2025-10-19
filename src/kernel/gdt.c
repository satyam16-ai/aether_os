// gdt.c - Global Descriptor Table setup for Aether OS
// Sets up a flat memory model with separate code and data segments

#include <stdint.h>
#include <printk.h>

// GDT Entry structure
struct gdt_entry {
    uint16_t limit_low;    // Lower 16 bits of limit
    uint16_t base_low;     // Lower 16 bits of base
    uint8_t  base_middle;  // Next 8 bits of base
    uint8_t  access;       // Access flags
    uint8_t  granularity;  // Granularity and upper 4 bits of limit
    uint8_t  base_high;    // Upper 8 bits of base
} __attribute__((packed));

// GDT Pointer structure for LGDT instruction
struct gdt_ptr {
    uint16_t limit;        // Size of GDT - 1
    uint32_t base;         // Address of GDT
} __attribute__((packed));

// GDT with 6 entries: null, kernel code, kernel data, user code, user data, TSS
#define GDT_ENTRIES 6
static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gdt_pointer;

// Segment selectors (indexes into GDT)
#define NULL_SEGMENT    0x00
#define KERNEL_CODE_SEG 0x08  // GDT entry 1
#define KERNEL_DATA_SEG 0x10  // GDT entry 2
#define USER_CODE_SEG   0x18  // GDT entry 3 (with RPL=3: 0x1B)
#define USER_DATA_SEG   0x20  // GDT entry 4 (with RPL=3: 0x23)

// Access byte flags
#define GDT_PRESENT     (1 << 7)
#define GDT_RING0       (0 << 5)
#define GDT_RING3       (3 << 5)
#define GDT_SYSTEM      (1 << 4)  // 0 = system, 1 = code/data
#define GDT_EXECUTABLE  (1 << 3)
#define GDT_DIRECTION   (1 << 2)  // 0 = grows up, 1 = grows down
#define GDT_WRITABLE    (1 << 1)
#define GDT_ACCESSED    (1 << 0)

// Granularity byte flags
#define GDT_4K_GRAN     (1 << 7)  // 0 = 1B granularity, 1 = 4KB granularity
#define GDT_32BIT       (1 << 6)  // 0 = 16-bit, 1 = 32-bit

// External assembly function to load GDT and flush segment registers
extern void gdt_flush(uint32_t gdt_ptr_addr);

// Helper function to set a GDT entry (now public for TSS)
void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    
    gdt[num].access      = access;
}

void gdt_init(void) {
    printk_info("Initializing Global Descriptor Table (GDT)");
    
    // Set up GDT pointer
    gdt_pointer.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gdt_pointer.base = (uint32_t)&gdt;
    
    printk("  GDT Base: 0x%X, Limit: %u bytes\n", gdt_pointer.base, gdt_pointer.limit + 1);
    
    // Null descriptor (required by CPU)
    gdt_set_gate(0, 0, 0, 0, 0);
    
    // Kernel code segment: base=0, limit=4GB, ring 0, executable, readable
    gdt_set_gate(1, 0, 0xFFFFFFFF, 
                 GDT_PRESENT | GDT_RING0 | GDT_SYSTEM | GDT_EXECUTABLE | GDT_WRITABLE,
                 GDT_4K_GRAN | GDT_32BIT);
    
    // Kernel data segment: base=0, limit=4GB, ring 0, writable
    gdt_set_gate(2, 0, 0xFFFFFFFF,
                 GDT_PRESENT | GDT_RING0 | GDT_SYSTEM | GDT_WRITABLE,
                 GDT_4K_GRAN | GDT_32BIT);
    
    // User code segment: base=0, limit=4GB, ring 3, executable, readable
    gdt_set_gate(3, 0, 0xFFFFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_SYSTEM | GDT_EXECUTABLE | GDT_WRITABLE,
                 GDT_4K_GRAN | GDT_32BIT);
    
    // User data segment: base=0, limit=4GB, ring 3, writable
    gdt_set_gate(4, 0, 0xFFFFFFFF,
                 GDT_PRESENT | GDT_RING3 | GDT_SYSTEM | GDT_WRITABLE,
                 GDT_4K_GRAN | GDT_32BIT);
    
    // Load GDT and flush segment registers
    gdt_flush((uint32_t)&gdt_pointer);
    
    printk("  [OK] GDT loaded with %d entries\n", GDT_ENTRIES);
    printk("       Kernel CS: 0x%02X, DS: 0x%02X\n", KERNEL_CODE_SEG, KERNEL_DATA_SEG);
    printk("       User CS: 0x%02X, DS: 0x%02X\n", USER_CODE_SEG | 3, USER_DATA_SEG | 3);
}

// Get current code segment selector
uint16_t gdt_get_kernel_cs(void) {
    return KERNEL_CODE_SEG;
}

// Get current data segment selector
uint16_t gdt_get_kernel_ds(void) {
    return KERNEL_DATA_SEG;
}