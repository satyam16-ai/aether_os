// scheduler.c - Process Scheduler (Round-Robin)
#include <scheduler.h>
#include <process.h>
#include <context.h>
#include <printk.h>
#include <timer.h>
#include <memory.h>
#include <tss.h>

// Ready queue (simple linked list)
static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;
static uint32_t ready_queue_count = 0;

// Scheduler state
static int scheduler_enabled = 0;
static uint32_t quantum_ticks = 10;  // Time slice per process (in timer ticks)

// Initialize scheduler
void scheduler_init(void) {
    printk_info("Initializing process scheduler");
    
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
    ready_queue_count = 0;
    scheduler_enabled = 0;
    
    printk("  Scheduling algorithm: Round-Robin\n");
    printk("  Time quantum: %d ticks (%d ms)\n", quantum_ticks, quantum_ticks * 10);
    printk("  [OK] Scheduler initialized (not yet enabled)\n");
}

// Add process to ready queue
void scheduler_add_process(process_t* process) {
    if (!process || process->state == PROCESS_STATE_TERMINATED) {
        return;
    }
    
    // Set process state to READY
    process->state = PROCESS_STATE_READY;
    process->next = NULL;
    process->prev = ready_queue_tail;
    
    // Add to tail of ready queue
    if (ready_queue_tail) {
        ready_queue_tail->next = process;
    } else {
        ready_queue_head = process;
    }
    ready_queue_tail = process;
    ready_queue_count++;
    
    printk("  Added process '%s' (PID %d) to ready queue\n", 
           process->name, process->pid);
}

// Remove process from ready queue
void scheduler_remove_process(process_t* process) {
    if (!process) {
        return;
    }
    
    // Update links
    if (process->prev) {
        process->prev->next = process->next;
    } else {
        ready_queue_head = process->next;
    }
    
    if (process->next) {
        process->next->prev = process->prev;
    } else {
        ready_queue_tail = process->prev;
    }
    
    process->next = NULL;
    process->prev = NULL;
    
    if (ready_queue_count > 0) {
        ready_queue_count--;
    }
}

// Pick next process to run (round-robin)
process_t* scheduler_schedule(void) {
    if (!ready_queue_head) {
        // No ready processes, return idle process (PID 0)
        return &process_table[0];
    }
    
    // Get first process from ready queue
    process_t* next = ready_queue_head;
    
    // Remove from queue
    scheduler_remove_process(next);
    
    // Set state to RUNNING
    next->state = PROCESS_STATE_RUNNING;
    next->quantum = quantum_ticks;
    
    return next;
}

// Enable scheduler
void scheduler_enable(void) {
    if (scheduler_enabled) {
        printk_warn("Scheduler already enabled");
        return;
    }
    
    printk_info("Enabling process scheduler");
    scheduler_enabled = 1;
}

// Disable scheduler
void scheduler_disable(void) {
    scheduler_enabled = 0;
    printk_info("Scheduler disabled");
}

// Check if scheduler is enabled
int scheduler_is_enabled(void) {
    return scheduler_enabled;
}

// Get ready queue count
uint32_t scheduler_get_ready_count(void) {
    return ready_queue_count;
}

// Called from timer interrupt
void scheduler_tick(void) {
    if (!scheduler_enabled || !current_process) {
        return;
    }
    
    // Update running time
    current_process->time_running++;
    
    // Check if quantum expired
    if (current_process->quantum > 0) {
        current_process->quantum--;
    }
    
    // Context switch if quantum expired and there are ready processes
    if (current_process->quantum == 0 && ready_queue_count > 0) {
        // Save current process
        process_t* old_process = current_process;
        
        // Add current process back to ready queue (if not terminated)
        if (old_process->state == PROCESS_STATE_RUNNING) {
            scheduler_add_process(old_process);
        }
        
        // Get next process to run
        process_t* next_process = scheduler_schedule();
        
        if (next_process && next_process != old_process) {
            // Debug logging
            printk("[SCHED] Switching: PID %d (%s, %s) -> PID %d (%s, %s)\n",
                   old_process->pid, old_process->name, 
                   old_process->is_kernel ? "kernel" : "user",
                   next_process->pid, next_process->name,
                   next_process->is_kernel ? "kernel" : "user");
            printk("        Old: ESP=0x%x EIP=0x%x DS=0x%x\n",
                   old_process->registers.esp, old_process->registers.eip,
                   old_process->registers.ds);
            printk("        New: ESP=0x%x EIP=0x%x DS=0x%x\n",
                   next_process->registers.esp, next_process->registers.eip,
                   next_process->registers.ds);
            
            // Update TSS kernel stack for the new process
            // When this process enters ring 3 and triggers interrupt/exception,
            // CPU will load esp0 from TSS for kernel stack
            uint32_t kernel_stack = (uint32_t)next_process->kernel_stack + 4096;
            tss_set_kernel_stack(kernel_stack);
            
            // Update current process pointer
            current_process = next_process;
            
            // Increment context switch counters
            old_process->context_switches++;
            next_process->context_switches++;
            
            // Perform the actual context switch
            context_switch(&old_process->registers, &next_process->registers);
        }
    } else if (current_process->quantum == 0) {
        // Reset quantum if no ready processes
        current_process->quantum = quantum_ticks;
    }
}

// Force immediate reschedule
void scheduler_yield(void) {
    if (!scheduler_enabled || !current_process) {
        return;
    }
    
    // If there are ready processes, yield to them
    if (ready_queue_count > 0) {
        // Save current process
        process_t* old_process = current_process;
        
        // Add current process back to ready queue (if not terminated)
        if (old_process->state == PROCESS_STATE_RUNNING) {
            old_process->quantum = quantum_ticks; // Reset quantum
            scheduler_add_process(old_process);
        }
        
        // Get next process to run
        process_t* next_process = scheduler_schedule();
        
        if (next_process && next_process != old_process) {
            // Update TSS kernel stack for the new process
            uint32_t kernel_stack = (uint32_t)next_process->kernel_stack + 4096;
            tss_set_kernel_stack(kernel_stack);
            
            // Update current process pointer
            current_process = next_process;
            
            // Increment context switch counters
            old_process->context_switches++;
            next_process->context_switches++;
            
            // Perform the actual context switch
            context_switch(&old_process->registers, &next_process->registers);
        }
    }
}

// Print scheduler statistics
void scheduler_print_stats(void) {
    printk("\n=== Scheduler Statistics ===\n");
    printk("Status: %s\n", scheduler_enabled ? "ENABLED" : "DISABLED");
    printk("Algorithm: Round-Robin\n");
    printk("Time Quantum: %d ticks\n", quantum_ticks);
    printk("Ready Queue: %d processes\n", ready_queue_count);
    
    if (current_process) {
        printk("Current Process: %s (PID %d)\n", 
               current_process->name, current_process->pid);
        printk("  Quantum Remaining: %d ticks\n", current_process->quantum);
        printk("  Total Runtime: %d ticks\n", current_process->time_running);
        printk("  Context Switches: %d\n", current_process->context_switches);
    }
    
    // List ready queue
    if (ready_queue_head) {
        printk("\nReady Queue:\n");
        process_t* p = ready_queue_head;
        int pos = 1;
        while (p) {
            printk("  %d. %s (PID %d, priority %d)\n", 
                   pos++, p->name, p->pid, p->priority);
            p = p->next;
        }
    }
}
