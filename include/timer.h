// timer.h - Programmable Interval Timer header
#ifndef AETHER_TIMER_H
#define AETHER_TIMER_H

#include <stdint.h>

// Initialize timer with specified frequency
void timer_init(uint32_t frequency_hz);

// Get current tick count and uptime
uint32_t timer_get_ticks(void);
uint32_t timer_get_uptime_seconds(void);
uint32_t timer_get_frequency(void);

// Sleep functions
void timer_sleep_ticks(uint32_t ticks);
void timer_sleep_ms(uint32_t milliseconds);

#endif // AETHER_TIMER_H