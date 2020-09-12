#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init();

void uart_put(uint8_t val);
uint8_t uart_get(uint8_t *val, uint16_t timeout);

#endif