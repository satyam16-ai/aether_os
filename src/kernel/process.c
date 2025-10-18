// process.c - Process Management Implementation
#include <process.h>
#include <memory.h>
#include <printk.h>
#include <timer.h>

// Process table and tracking
process_t process_table[MAX_PROCESSES];
process_t* current_process = NULL;
uint32_t next_pid = 0;

// String utilities (simple implementations)
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

static char* strncpy_local(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

// Initialize process subsystem
void process_init(void) {
    printk_info("Initializing process management subsystem");
    
    // Clear process table
    memset(process_table, 0, sizeof(process_table));
    
    // Mark all processes as terminated (free)
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROCESS_STATE_TERMINATED;
        process_table[i].pid = i;
    }
    
    // Create kernel idle process (PID 0)
    process_t* idle = &process_table[0];
    idle->pid = 0;
    strncpy_local(idle->name, "kernel_idle", 31);
    idle->name[31] = '\0';
    idle->state = PROCESS_STATE_RUNNING;
    idle->priority = PROCESS_PRIORITY_IDLE;
    idle->quantum = 1;
    idle->page_directory = paging_get_current_directory();
    idle->kernel_stack = 0;  // Uses current stack
    idle->user_stack = 0;
    idle->parent = NULL;
    idle->num_children = 0;
    idle->next = NULL;
    idle->prev = NULL;
    idle->time_created = timer_get_ticks();
    idle->time_running = 0;
    idle->context_switches = 0;
    idle->exit_code = 0;
    
    // Set as current process
    current_process = idle;
    next_pid = 1;
    
    printk("  [OK] Process subsystem initialized\n");
    printk("       Idle process (PID 0) created\n");
}

// Allocate a new PID
uint32_t process_allocate_pid(void) {
    // Simple linear search for free slot
    for (uint32_t i = next_pid; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STATE_TERMINATED) {
            next_pid = i + 1;
            return i;
        }
    }
    
    // Wrap around
    for (uint32_t i = 1; i < next_pid; i++) {
        if (process_table[i].state == PROCESS_STATE_TERMINATED) {
            next_pid = i + 1;
            return i;
        }
    }
    
    // No free PIDs
    return 0xFFFFFFFF;
}

// Free a PID
void process_free_pid(uint32_t pid) {
    if (pid < MAX_PROCESSES) {
        process_table[pid].state = PROCESS_STATE_TERMINATED;
    }
}

// Create a new process
process_t* process_create(const char* name, void (*entry_point)(void), 
                          process_priority_t priority) {
    // Allocate PID
    uint32_t pid = process_allocate_pid();
    if (pid == 0xFFFFFFFF) {
        printk_error("Failed to create process: no free PIDs");
        return NULL;
    }
    
    process_t* process = &process_table[pid];
    
    // Initialize process structure
    memset(process, 0, sizeof(process_t));
    process->pid = pid;
    
    // Copy name
    if (name) {
        strncpy_local(process->name, name, 31);
        process->name[31] = '\0';
    } else {
        strncpy_local(process->name, "unnamed", 31);
    }
    
    // Set initial state
    process->state = PROCESS_STATE_NEW;
    process->priority = priority;
    process->quantum = 10;  // Default time slice
    
    // Allocate kernel stack
    process->kernel_stack = (uint32_t)kmalloc(KERNEL_STACK_SIZE);
    if (!process->kernel_stack) {
        printk_error("Failed to allocate kernel stack for process %d", pid);
        process_free_pid(pid);
        return NULL;
    }
    
    // Allocate user stack (for now, same as kernel)
    process->user_stack = (uint32_t)kmalloc(USER_STACK_SIZE);
    if (!process->user_stack) {
        printk_error("Failed to allocate user stack for process %d", pid);
        kfree((void*)process->kernel_stack);
        process_free_pid(pid);
        return NULL;
    }
    
    // Set up initial stack (stack grows downward)
    process->registers.esp = process->kernel_stack + KERNEL_STACK_SIZE - 4;
    process->registers.ebp = process->registers.esp;
    
    // Set entry point
    process->registers.eip = (uint32_t)entry_point;
    
    // Set initial flags (interrupts enabled)
    process->registers.eflags = 0x202;
    
    // Use current page directory for now
    process->page_directory = paging_get_current_directory();
    
    // Set parent as current process
    process->parent = current_process;
    if (current_process && current_process->num_children < 16) {
        current_process->children[current_process->num_children++] = process;
    }
    
    process->num_children = 0;
    
    // Initialize scheduling info
    process->next = NULL;
    process->prev = NULL;
    process->time_created = timer_get_ticks();
    process->time_running = 0;
    process->context_switches = 0;
    process->exit_code = 0;
    
    // Transition to READY state
    process->state = PROCESS_STATE_READY;
    
    printk("  Created process '%s' (PID %d, priority %d)\n", 
           process->name, process->pid, process->priority);
    
    return process;
}

// Destroy a process
void process_destroy(process_t* process) {
    if (!process || process->pid == 0) {
        printk_error("Cannot destroy null or idle process");
        return;
    }
    
    // Remove from parent's child list
    if (process->parent) {
        for (uint32_t i = 0; i < process->parent->num_children; i++) {
            if (process->parent->children[i] == process) {
                // Shift remaining children
                for (uint32_t j = i; j < process->parent->num_children - 1; j++) {
                    process->parent->children[j] = process->parent->children[j + 1];
                }
                process->parent->num_children--;
                break;
            }
        }
    }
    
    // Reparent children to init (PID 1) or idle (PID 0)
    process_t* new_parent = (next_pid > 1) ? &process_table[1] : &process_table[0];
    for (uint32_t i = 0; i < process->num_children; i++) {
        process->children[i]->parent = new_parent;
        if (new_parent->num_children < 16) {
            new_parent->children[new_parent->num_children++] = process->children[i];
        }
    }
    
    // Free stacks
    if (process->kernel_stack) {
        kfree((void*)process->kernel_stack);
    }
    if (process->user_stack) {
        kfree((void*)process->user_stack);
    }
    
    // Mark as terminated
    process->state = PROCESS_STATE_TERMINATED;
    
    printk("  Destroyed process '%s' (PID %d)\n", process->name, process->pid);
}

// Exit current process
void process_exit(int exit_code) {
    if (!current_process) {
        return;
    }
    
    current_process->exit_code = exit_code;
    current_process->state = PROCESS_STATE_TERMINATED;
    
    printk("  Process '%s' (PID %d) exited with code %d\n", 
           current_process->name, current_process->pid, exit_code);
    
    // TODO: Switch to next ready process
}

// Set process state
void process_set_state(process_t* process, process_state_t state) {
    if (!process) return;
    process->state = state;
}

// Get state name as string
const char* process_get_state_name(process_state_t state) {
    switch (state) {
        case PROCESS_STATE_NEW:        return "NEW";
        case PROCESS_STATE_READY:      return "READY";
        case PROCESS_STATE_RUNNING:    return "RUNNING";
        case PROCESS_STATE_BLOCKED:    return "BLOCKED";
        case PROCESS_STATE_TERMINATED: return "TERMINATED";
        default:                       return "UNKNOWN";
    }
}

// Get current process
process_t* process_get_current(void) {
    return current_process;
}

// Set current process
void process_set_current(process_t* process) {
    current_process = process;
}

// Get process by PID
process_t* process_get_by_pid(uint32_t pid) {
    if (pid >= MAX_PROCESSES) {
        return NULL;
    }
    
    process_t* process = &process_table[pid];
    if (process->state == PROCESS_STATE_TERMINATED) {
        return NULL;
    }
    
    return process;
}

// Print process information
void process_print_info(process_t* process) {
    if (!process) {
        printk("  Process: NULL\n");
        return;
    }
    
    printk("  Process Information:\n");
    printk("    PID:      %d\n", process->pid);
    printk("    Name:     %s\n", process->name);
    printk("    State:    %s\n", process_get_state_name(process->state));
    printk("    Priority: %d\n", process->priority);
    printk("    Parent:   %d\n", process->parent ? process->parent->pid : -1);
    printk("    Children: %d\n", process->num_children);
    printk("    Runtime:  %d ticks\n", process->time_running);
    printk("    Switches: %d\n", process->context_switches);
}

// List all processes
void process_list_all(void) {
    printk("\n=== Process List ===\n");
    printk("PID  Name                State       Priority  Runtime\n");
    printk("---  ------------------  ----------  --------  -------\n");
    
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_STATE_TERMINATED) {
            printk("%-4d %-18s  %-10s  %-8d  %d\n",
                   process_table[i].pid,
                   process_table[i].name,
                   process_get_state_name(process_table[i].state),
                   process_table[i].priority,
                   process_table[i].time_running);
            count++;
        }
    }
    
    printk("\nTotal processes: %d\n", count);
}
