#ifndef GPIO_H
#define GPIO_H

#include "driver.h"

#include <stdint.h>

enum gpio_mode {
  GPIO_INPUT = (1 << 1),
  GPIO_OUTPUT = (1 << 2),

  GPIO_PP = (1 << 3),
  GPIO_OD = (1 << 4),

  GPIO_AF = (1 << 5),
};

void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint32_t mode);

void gpio_set(GPIO_TypeDef *port, uint8_t pin);
void gpio_reset(GPIO_TypeDef *port, uint8_t pin);

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);

#endif