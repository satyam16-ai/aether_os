// memory.c - Basic Memory Manager for Aether OS
#include <memory.h>
#include <printk.h>
#include <stdint.h>
#include <stddef.h>

// Heap management globals
static memory_block_t* heap_start = NULL;
static memory_block_t* heap_end = NULL;
static uint32_t total_heap_size = 0;
static int heap_initialized = 0;

void memory_init(void) {
    // Initialize the kernel heap at 2MB
    heap_start = (memory_block_t*)KERNEL_HEAP_START;
    total_heap_size = KERNEL_HEAP_SIZE;
    
    // Create the initial free block
    heap_start->size = KERNEL_HEAP_SIZE - sizeof(memory_block_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_end = heap_start;
    heap_initialized = 1;
    
    printk_info("Memory manager initialized");
    printk("  Heap start: 0x%x\n", (uint32_t)heap_start);
    printk("  Heap size:  %u KB\n", KERNEL_HEAP_SIZE / 1024);
}

// Find a free block of at least the requested size
static memory_block_t* find_free_block(size_t size) {
    memory_block_t* current = heap_start;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Split a block if it's larger than needed
static void split_block(memory_block_t* block, size_t size) {
    if (block->size <= size + sizeof(memory_block_t)) {
        return; // Not worth splitting
    }
    
    memory_block_t* new_block = (memory_block_t*)((uint8_t*)block + sizeof(memory_block_t) + size);
    new_block->size = block->size - size - sizeof(memory_block_t);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    } else {
        heap_end = new_block;
    }
    
    block->next = new_block;
    block->size = size;
}

// Merge adjacent free blocks
static void merge_free_blocks(memory_block_t* block) {
    // Merge with next block if it's free
    if (block->next && block->next->is_free) {
        block->size += sizeof(memory_block_t) + block->next->size;
        if (block->next->next) {
            block->next->next->prev = block;
        } else {
            heap_end = block;
        }
        block->next = block->next->next;
    }
    
    // Merge with previous block if it's free
    if (block->prev && block->prev->is_free) {
        block->prev->size += sizeof(memory_block_t) + block->size;
        if (block->next) {
            block->next->prev = block->prev;
        } else {
            heap_end = block->prev;
        }
        block->prev->next = block->next;
    }
}

void* kmalloc(size_t size) {
    if (!heap_initialized || size == 0) {
        return NULL;
    }
    
    // Align to 4-byte boundary
    size = (size + 3) & ~3;
    
    memory_block_t* block = find_free_block(size);
    if (!block) {
        return NULL; // Out of memory
    }
    
    split_block(block, size);
    block->is_free = 0;
    
    return (uint8_t*)block + sizeof(memory_block_t);
}

void kfree(void* ptr) {
    if (!ptr || !heap_initialized) {
        return;
    }
    
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    // Validate the block is within our heap
    if ((uint8_t*)block < (uint8_t*)heap_start || 
        (uint8_t*)block >= (uint8_t*)heap_start + total_heap_size) {
        return;
    }
    
    block->is_free = 1;
    merge_free_blocks(block);
}

void* krealloc(void* ptr, size_t new_size) {
    if (!ptr) {
        return kmalloc(new_size);
    }
    
    if (new_size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    memory_block_t* block = (memory_block_t*)((uint8_t*)ptr - sizeof(memory_block_t));
    
    if (block->size >= new_size) {
        return ptr; // Current block is large enough
    }
    
    void* new_ptr = kmalloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    
    memcpy(new_ptr, ptr, block->size < new_size ? block->size : new_size);
    kfree(ptr);
    
    return new_ptr;
}

void* kcalloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = kmalloc(total_size);
    
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

void memory_get_stats(memory_stats_t* stats) {
    if (!stats || !heap_initialized) {
        return;
    }
    
    stats->total_memory = total_heap_size;
    stats->used_memory = 0;
    stats->free_memory = 0;
    stats->allocated_blocks = 0;
    stats->free_blocks = 0;
    
    memory_block_t* current = heap_start;
    while (current) {
        if (current->is_free) {
            stats->free_memory += current->size;
            stats->free_blocks++;
        } else {
            stats->used_memory += current->size;
            stats->allocated_blocks++;
        }
        current = current->next;
    }
}

void memory_print_stats(void) {
    memory_stats_t stats;
    memory_get_stats(&stats);
    
    printk("\nMemory Statistics:\n");
    printk("  Total heap:      %u KB\n", stats.total_memory / 1024);
    printk("  Used memory:     %u KB (%u%%)\n", 
           stats.used_memory / 1024,
           (stats.used_memory * 100) / stats.total_memory);
    printk("  Free memory:     %u KB (%u%%)\n",
           stats.free_memory / 1024,
           (stats.free_memory * 100) / stats.total_memory);
    printk("  Allocated blocks: %u\n", stats.allocated_blocks);
    printk("  Free blocks:     %u\n", stats.free_blocks);
}

void memory_dump_heap(void) {
    if (!heap_initialized) {
        printk("Heap not initialized\n");
        return;
    }
    
    printk("\nHeap Dump:\n");
    memory_block_t* current = heap_start;
    int block_num = 0;
    
    while (current && block_num < 20) { // Limit output
        printk("  Block %d: addr=0x%x, size=%u, %s\n",
               block_num,
               (uint32_t)current,
               current->size,
               current->is_free ? "FREE" : "USED");
        current = current->next;
        block_num++;
    }
    
    if (current) {
        printk("  ... (more blocks)\n");
    }
}

uint32_t memory_get_total(void) {
    // For now, return a fixed amount. Later we can detect from BIOS/UEFI
    return 64 * 1024 * 1024; // 64MB
}

uint32_t memory_get_available(void) {
    memory_stats_t stats;
    memory_get_stats(&stats);
    return stats.free_memory;
}

// Memory utility functions
void* memset(void* ptr, int value, size_t size) {
    uint8_t* p = (uint8_t*)ptr;
    while (size--) {
        *p++ = (uint8_t)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    
    while (size--) {
        *d++ = *s++;
    }
    
    return dest;
}

int memcmp(const void* ptr1, const void* ptr2, size_t size) {
    const uint8_t* p1 = (const uint8_t*)ptr1;
    const uint8_t* p2 = (const uint8_t*)ptr2;
    
    while (size--) {
        if (*p1 != *p2) {
            return (*p1 < *p2) ? -1 : 1;
        }
        p1++;
        p2++;
    }
    
    return 0;
}