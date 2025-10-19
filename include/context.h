// context.h - CPU Context Management and Switching
#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <process.h>

// Assembly context switching functions
extern void context_switch(registers_t* old_regs, registers_t* new_regs);
extern uint32_t read_esp(void);
extern uint32_t read_ebp(void);
extern uint32_t read_eip(void);

// High-level context management
void context_init(void);
void switch_to_process(process_t* new_process);
void save_current_context(void);

// Helper functions to read CPU state
static inline uint32_t read_eflags(void) {
    uint32_t eflags;
    __asm__ volatile("pushfd; pop %0" : "=r"(eflags));
    return eflags;
}

static inline void write_eflags(uint32_t eflags) {
    __asm__ volatile("push %0; popfd" : : "r"(eflags));
}

static inline uint32_t read_cr3(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void write_cr3(uint32_t cr3) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(cr3));
}

static inline uint16_t read_ds(void) {
    uint16_t ds;
    __asm__ volatile("mov %%ds, %0" : "=r"(ds));
    return ds;
}

static inline uint16_t read_es(void) {
    uint16_t es;
    __asm__ volatile("mov %%es, %0" : "=r"(es));
    return es;
}

static inline uint16_t read_fs(void) {
    uint16_t fs;
    __asm__ volatile("mov %%fs, %0" : "=r"(fs));
    return fs;
}

static inline uint16_t read_gs(void) {
    uint16_t gs;
    __asm__ volatile("mov %%gs, %0" : "=r"(gs));
    return gs;
}

static inline uint16_t read_ss(void) {
    uint16_t ss;
    __asm__ volatile("mov %%ss, %0" : "=r"(ss));
    return ss;
}

#endif // CONTEXT_H
