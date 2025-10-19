#ifndef USERMODE_H
#define USERMODE_H

#include <stdint.h>
#include <process.h>

// User mode entry function
void enter_user_mode(uint32_t entry_point, uint32_t user_stack);

// Set up a process to run in user mode
void process_setup_user_mode(process_t* process, void (*entry_point)(void));

// User mode test functions
void user_mode_test_1(void);
void user_mode_test_2(void);

// Helper to check current privilege level
uint32_t get_current_privilege_level(void);

#endif // USERMODE_H
