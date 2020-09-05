#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stdint.h>

#include "cc25xx.h"
#include "uart.h"

#ifdef DEBUG_OUTPUT
#define debug_put(c) uart_put(c)
#define debug_print(str) uart_print(str)
#define debug_printf(fmt, ...) uart_printf(fmt, __VA_ARGS__)

uint16_t debug_vsnprintf(char *buffer, uint16_t buffer_len, char *fmt, va_list va);
uint16_t debug_snprintf(char *buffer, uint16_t buffer_len, char *fmt, ...);
#else
#define debug_put(c)
#define debug_print(str)
#define debug_printf(fmt, ...)
#endif

#endif