// paging.c - Virtual Memory Management Implementation
#include <paging.h>
#include <memory.h>
#include <printk.h>
#include <stdint.h>
#include <stddef.h>

// Kernel page directory (global)
static page_directory_t* kernel_directory = NULL;
static page_directory_t* current_directory = NULL;

// Assembly helper to load page directory (defined at end of file)
extern void paging_load_directory(uint32_t* page_directory_physical);
extern void paging_enable_hw(void);

void paging_init(void) {
    printk_info("Initializing Virtual Memory (Paging)");
    
    // Allocate kernel page directory
    kernel_directory = (page_directory_t*)kmalloc(sizeof(page_directory_t));
    if (!kernel_directory) {
        printk_error("Failed to allocate kernel page directory!");
        return;
    }
    
    // Clear page directory
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        kernel_directory->entries[i] = 0;
    }
    
    printk("  Kernel page directory allocated at: %p\n", kernel_directory);
    
    // Identity map the first 4MB (0x00000000 - 0x00400000)
    // This covers the kernel code and data
    printk("  Identity mapping first 4MB (kernel space)...\n");
    paging_identity_map(kernel_directory, 0x00000000, 0x00400000, 
                       PAGE_PRESENT | PAGE_WRITE);
    
    // Identity map VGA text buffer (0xB8000 - 0xB9000)
    printk("  Mapping VGA text buffer (0xB8000)...\n");
    paging_map_page(kernel_directory, 0xB8000, 0xB8000, 
                   PAGE_PRESENT | PAGE_WRITE);
    
    // Set current directory
    current_directory = kernel_directory;
    
    printk("  [OK] Page tables configured\n");
    printk("  [OK] Identity mapping complete\n");
}

void paging_enable(void) {
    if (!kernel_directory) {
        printk_error("Cannot enable paging: kernel directory not initialized!");
        return;
    }
    
    printk_info("Enabling hardware paging...");
    
    // Load page directory address into CR3
    uint32_t pd_physical = (uint32_t)kernel_directory;
    printk("  Loading page directory (CR3 = %p)\n", pd_physical);
    paging_load_directory((uint32_t*)pd_physical);
    
    // Enable paging by setting bit 31 in CR0
    paging_enable_hw();
    
    printk("  [OK] Paging enabled! Virtual memory active.\n");
}

void paging_identity_map(page_directory_t* dir, uint32_t start, 
                         uint32_t end, uint32_t flags) {
    // Align addresses to page boundaries
    start = start & 0xFFFFF000;
    end = PAGE_ALIGN(end);
    
    printk("    Mapping %p -> %p (identity)\n", start, end);
    
    // Map each page in the range
    for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
        paging_map_page(dir, addr, addr, flags);
    }
}

void paging_map_page(page_directory_t* dir, uint32_t virtual_addr, 
                     uint32_t physical_addr, uint32_t flags) {
    // Get page directory and table indices
    uint32_t dir_index = paging_directory_index(virtual_addr);
    uint32_t table_index = paging_table_index(virtual_addr);
    
    // Check if page table exists
    page_directory_entry_t* pde = &dir->entries[dir_index];
    page_table_t* page_table;
    
    if (!paging_is_present(*pde)) {
        // Allocate new page table
        page_table = (page_table_t*)kmalloc(sizeof(page_table_t));
        if (!page_table) {
            printk_error("Failed to allocate page table!");
            return;
        }
        
        // Clear page table
        for (int i = 0; i < PAGE_ENTRIES; i++) {
            page_table->entries[i] = 0;
        }
        
        // Add page table to directory
        *pde = paging_create_pde((uint32_t)page_table, PAGE_PRESENT | PAGE_WRITE | flags);
    } else {
        // Get existing page table
        page_table = (page_table_t*)paging_get_address(*pde);
    }
    
    // Set page table entry
    page_table->entries[table_index] = paging_create_pte(physical_addr, PAGE_PRESENT | flags);
}

void paging_unmap_page(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t dir_index = paging_directory_index(virtual_addr);
    uint32_t table_index = paging_table_index(virtual_addr);
    
    page_directory_entry_t* pde = &dir->entries[dir_index];
    
    if (!paging_is_present(*pde)) {
        return; // Page table doesn't exist
    }
    
    page_table_t* page_table = (page_table_t*)paging_get_address(*pde);
    page_table->entries[table_index] = 0; // Clear entry
    
    // Invalidate TLB for this page
    __asm__ volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

uint32_t paging_get_physical_address(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t dir_index = paging_directory_index(virtual_addr);
    uint32_t table_index = paging_table_index(virtual_addr);
    uint32_t offset = paging_page_offset(virtual_addr);
    
    page_directory_entry_t* pde = &dir->entries[dir_index];
    
    if (!paging_is_present(*pde)) {
        return 0; // Not mapped
    }
    
    page_table_t* page_table = (page_table_t*)paging_get_address(*pde);
    page_table_entry_t pte = page_table->entries[table_index];
    
    if (!paging_is_present(pte)) {
        return 0; // Not mapped
    }
    
    return paging_get_address(pte) + offset;
}

page_directory_t* paging_get_current_directory(void) {
    return current_directory;
}

void paging_switch_directory(page_directory_t* dir) {
    current_directory = dir;
    paging_load_directory((uint32_t*)((uint32_t)dir));
}

// Page fault handler (called from interrupt handler)
void page_fault_handler(void) {
    // Get faulting address from CR2
    uint32_t faulting_address;
    __asm__ volatile("mov %%cr2, %0" : "=r"(faulting_address));
    
    printk_error("Page Fault!");
    printk("  Faulting address: 0x%08X\n", faulting_address);
    printk("  This means the address is not mapped or access was denied.\n");
    
    // For now, just halt
    // TODO: Handle page faults properly (allocate pages, swap, etc.)
    __asm__ volatile("cli; hlt");
}

// Assembly helpers for paging control
__asm__(
    ".global paging_load_directory\n"
    "paging_load_directory:\n"
    "   mov 4(%esp), %eax\n"      // Get page directory address from stack
    "   mov %eax, %cr3\n"          // Load into CR3
    "   ret\n"
);

__asm__(
    ".global paging_enable_hw\n"
    "paging_enable_hw:\n"
    "   mov %cr0, %eax\n"          // Read CR0
    "   or $0x80000000, %eax\n"    // Set bit 31 (PG - Paging Enable)
    "   mov %eax, %cr0\n"          // Write back to CR0
    "   ret\n"
);
