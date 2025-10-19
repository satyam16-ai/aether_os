// scheduler.h - Process Scheduler
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <process.h>

// Scheduler initialization
void scheduler_init(void);

// Process queue management
void scheduler_add_process(process_t* process);
void scheduler_remove_process(process_t* process);

// Scheduling
process_t* scheduler_schedule(void);
void scheduler_tick(void);
void scheduler_yield(void);

// Scheduler control
void scheduler_enable(void);
void scheduler_disable(void);
int scheduler_is_enabled(void);

// Statistics
uint32_t scheduler_get_ready_count(void);
void scheduler_print_stats(void);

#endif // SCHEDULER_H
