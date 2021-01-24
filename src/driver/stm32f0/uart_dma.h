#ifndef UART_DMA_H
#define UART_DMA_H

#include <stdint.h>

#include "driver.h"
#include "uart.h"

void uart_dma_init();

uint8_t uart_dma_start(uint8_t *data, uint16_t len);

void uart_dma_print(const char *str);
void uart_dma_printf(char *fmt, ...);

#endif