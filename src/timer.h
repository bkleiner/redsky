#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "cc25xx.h"

void clock_init();

void timer_init();

void timer_timeout_set_us(uint32_t us);
void timer_timeout_set_ms(uint32_t ms);
uint8_t timer_timeout();

void timer3_isr(void) __interrupt(T3_VECTOR);

#endif