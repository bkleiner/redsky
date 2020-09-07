#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stdint.h>

#include "cc25xx.h"
#include "uart_dma.h"

#ifdef DEBUG_OUTPUT
#define debug_print(str) uart_dma_print(str)
#define debug_printf(fmt, ...) uart_dma_printf(fmt, __VA_ARGS__)

uint16_t debug_vsnprintf(char *buffer, uint16_t buffer_len, char *fmt, va_list va);
uint16_t debug_snprintf(char *buffer, uint16_t buffer_len, char *fmt, ...);
#else
#define debug_print(str)
#define debug_printf(fmt, ...)
#endif

#endif