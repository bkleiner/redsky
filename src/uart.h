#ifndef UART_H
#define UART_H

#include "cc25xx.h"
#include "debug.h"

#include <stdint.h>

void uart_init();

uint8_t uart_start(uint8_t *data, uint16_t len);
uint8_t uart_get(uint8_t *val, uint16_t timeout);

void uart_print(const char *str);
void uart_printf(char *fmt, ...);

void uart_dma_isr();

#endif