#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "cc25xx.h"

void uart_init();
void uart_put(uint8_t c);
void uart_print(const char *str);
void uart_flush();

void uart_dma_isr(void) __interrupt(DMA_VECTOR);

#endif