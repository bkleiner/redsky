#ifndef DEBUG_H
#define DEBUG_H

#include "uart.h"

#define debug_put(c) uart_put(c)
#define debug_print(str) uart_print(str)
#define debug_flush() uart_flush()

#endif