#pragma once

#include <stdint.h>

void uart_init();
void uart_put(uint8_t c);
void uart_print(const char *str);