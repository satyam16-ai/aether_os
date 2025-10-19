// process.h - Process Control Block and Process Management
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <paging.h>

// Maximum number of processes
#define MAX_PROCESSES       256
#define KERNEL_STACK_SIZE   4096    // 4KB kernel stack per process
#define USER_STACK_SIZE     4096    // 4KB user stack per process

// Process states
typedef enum {
    PROCESS_STATE_NEW = 0,          // Just created
    PROCESS_STATE_READY,            // Ready to run
    PROCESS_STATE_RUNNING,          // Currently executing
    PROCESS_STATE_BLOCKED,          // Waiting for I/O or event
    PROCESS_STATE_TERMINATED        // Finished execution
} process_state_t;

// Process priority levels
typedef enum {
    PROCESS_PRIORITY_IDLE = 0,      // Idle/background tasks
    PROCESS_PRIORITY_LOW = 1,       // Low priority
    PROCESS_PRIORITY_NORMAL = 2,    // Normal priority (default)
    PROCESS_PRIORITY_HIGH = 3,      // High priority
    PROCESS_PRIORITY_REALTIME = 4   // Real-time priority
} process_priority_t;

// CPU register state for context switching
typedef struct {
    // General purpose registers
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    
    // Segment registers
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
    
    // Special registers
    uint32_t eip;       // Instruction pointer
    uint32_t eflags;    // CPU flags
    uint32_t cr3;       // Page directory base (for virtual memory)
} registers_t;

// Process Control Block (PCB)
typedef struct process {
    // Process identification
    uint32_t pid;                   // Process ID
    char name[32];                  // Process name
    
    // Process state
    process_state_t state;          // Current state
    process_priority_t priority;    // Scheduling priority
    uint32_t quantum;               // Time slice remaining (in ticks)
    
    // CPU context
    registers_t registers;          // Saved CPU state
    
    // Memory management
    page_directory_t* page_directory;   // Virtual address space
    uint32_t kernel_stack;          // Kernel stack pointer
    uint32_t user_stack;            // User stack pointer
    uint8_t is_kernel;              // 1 = kernel mode, 0 = user mode
    
    // Parent/child relationships
    struct process* parent;         // Parent process
    struct process* children[16];   // Child processes (max 16)
    uint32_t num_children;
    
    // Scheduling
    struct process* next;           // Next process in queue
    struct process* prev;           // Previous process in queue
    
    // Statistics
    uint32_t time_created;          // Tick when process was created
    uint32_t time_running;          // Total ticks spent running
    uint32_t context_switches;      // Number of context switches
    
    // Exit status
    int exit_code;                  // Return value when process exits
} process_t;

// Process table and current process
extern process_t process_table[MAX_PROCESSES];
extern process_t* current_process;
extern uint32_t next_pid;

// Process management functions
void process_init(void);
process_t* process_create(const char* name, void (*entry_point)(void), 
                          process_priority_t priority);
void process_destroy(process_t* process);
void process_exit(int exit_code);

// Process state management
void process_set_state(process_t* process, process_state_t state);
const char* process_get_state_name(process_state_t state);

// Process scheduling helpers
process_t* process_get_current(void);
void process_set_current(process_t* process);
process_t* process_get_by_pid(uint32_t pid);

// PID allocation
uint32_t process_allocate_pid(void);
void process_free_pid(uint32_t pid);

// Process information
void process_print_info(process_t* process);
void process_list_all(void);

#endif // PROCESS_H
