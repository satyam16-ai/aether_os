// paging.h - Virtual Memory Management (Paging)
#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <memory.h>  // For PAGE_SIZE definition

// Page alignment macro
#define PAGE_ALIGN(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

// Number of entries in page directory/table
#define PAGE_ENTRIES        1024

// Page directory/table entry flags
#define PAGE_PRESENT        0x001   // Page is present in memory
#define PAGE_WRITE          0x002   // Page is writable
#define PAGE_USER           0x004   // Page is accessible from user mode
#define PAGE_WRITETHROUGH   0x008   // Write-through caching
#define PAGE_NOCACHE        0x010   // Disable caching
#define PAGE_ACCESSED       0x020   // Page has been accessed
#define PAGE_DIRTY          0x040   // Page has been written to
#define PAGE_SIZE_4MB       0x080   // 4MB pages (only in page directory)
#define PAGE_GLOBAL         0x100   // Global page (TLB)

// Page directory entry (PDE) - points to a page table
typedef uint32_t page_directory_entry_t;

// Page table entry (PTE) - points to a physical page
typedef uint32_t page_table_entry_t;

// Page directory (1024 entries, each pointing to a page table)
typedef struct {
    page_directory_entry_t entries[PAGE_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) page_directory_t;

// Page table (1024 entries, each pointing to a 4KB page)
typedef struct {
    page_table_entry_t entries[PAGE_ENTRIES];
} __attribute__((aligned(PAGE_SIZE))) page_table_t;

// Virtual address breakdown
typedef struct {
    uint32_t offset     : 12;  // Offset within page (bits 0-11)
    uint32_t table      : 10;  // Page table index (bits 12-21)
    uint32_t directory  : 10;  // Page directory index (bits 22-31)
} virtual_address_t;

// Physical address breakdown
typedef struct {
    uint32_t offset     : 12;  // Offset within page
    uint32_t frame      : 20;  // Physical frame number
} physical_address_t;

// Paging initialization
void paging_init(void);

// Enable/disable paging
void paging_enable(void);
void paging_disable(void);

// Page directory management
page_directory_t* paging_create_directory(void);
void paging_destroy_directory(page_directory_t* dir);
void paging_switch_directory(page_directory_t* dir);
page_directory_t* paging_get_current_directory(void);

// Page mapping functions
void paging_map_page(page_directory_t* dir, uint32_t virtual_addr, 
                     uint32_t physical_addr, uint32_t flags);
void paging_unmap_page(page_directory_t* dir, uint32_t virtual_addr);
uint32_t paging_get_physical_address(page_directory_t* dir, uint32_t virtual_addr);

// Identity mapping (virtual = physical)
void paging_identity_map(page_directory_t* dir, uint32_t start, 
                         uint32_t end, uint32_t flags);

// Page fault handler
void page_fault_handler(void);

// Helper functions
static inline uint32_t paging_directory_index(uint32_t virtual_addr) {
    return (virtual_addr >> 22) & 0x3FF;
}

static inline uint32_t paging_table_index(uint32_t virtual_addr) {
    return (virtual_addr >> 12) & 0x3FF;
}

static inline uint32_t paging_page_offset(uint32_t virtual_addr) {
    return virtual_addr & 0xFFF;
}

// Get physical frame number from address
static inline uint32_t paging_frame_number(uint32_t physical_addr) {
    return physical_addr >> 12;
}

// Create page table entry
static inline page_table_entry_t paging_create_pte(uint32_t physical_addr, uint32_t flags) {
    return (physical_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// Create page directory entry
static inline page_directory_entry_t paging_create_pde(uint32_t page_table_addr, uint32_t flags) {
    return (page_table_addr & 0xFFFFF000) | (flags & 0xFFF);
}

// Extract physical address from PTE/PDE
static inline uint32_t paging_get_address(uint32_t entry) {
    return entry & 0xFFFFF000;
}

// Extract flags from PTE/PDE
static inline uint32_t paging_get_flags(uint32_t entry) {
    return entry & 0xFFF;
}

// Check if page is present
static inline int paging_is_present(uint32_t entry) {
    return entry & PAGE_PRESENT;
}

#endif // PAGING_H
