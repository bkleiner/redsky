#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init();

void timer_timeout_set_100us(uint32_t us100);
void timer_timeout_set_ms(uint16_t ms);

uint8_t timer_timeout();

#endif