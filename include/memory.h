#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Memory layout constants
#define KERNEL_START        0x100000    // 1MB - where kernel is loaded
#define KERNEL_HEAP_START   0x200000    // 2MB - start of kernel heap
#define KERNEL_HEAP_SIZE    0x400000    // 4MB - kernel heap size
#define PAGE_SIZE           0x1000      // 4KB pages

// Memory block header for simple heap allocator
typedef struct memory_block {
    size_t size;
    int is_free;
    struct memory_block* next;
    struct memory_block* prev;
} memory_block_t;

// Memory statistics
typedef struct {
    uint32_t total_memory;
    uint32_t used_memory;
    uint32_t free_memory;
    uint32_t allocated_blocks;
    uint32_t free_blocks;
} memory_stats_t;

// Function prototypes
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t new_size);
void* kcalloc(size_t num, size_t size);

// Memory information and debugging
void memory_get_stats(memory_stats_t* stats);
void memory_print_stats(void);
void memory_dump_heap(void);

// Physical memory management (basic)
uint32_t memory_get_total(void);
uint32_t memory_get_available(void);

// Memory utilities
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
int memcmp(const void* ptr1, const void* ptr2, size_t size);

#endif // MEMORY_H