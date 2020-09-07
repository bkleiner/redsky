#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

void delay_ms(uint16_t ms);
void delay_us(uint16_t us);

void delay_45_nop();

#endif