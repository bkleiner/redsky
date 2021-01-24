#ifndef GPIO_H
#define GPIO_H

#include "driver.h"

#include <stdint.h>

typedef struct {
  GPIO_TypeDef *port;
  uint8_t index;
} gpio_pin_def_t;

#define MAKE_PIN_DEF(port, pin) extern gpio_pin_def_t P##port##pin;
#include "gpio_pin.h.in"
#undef MAKE_PIN_DEF

enum gpio_mode {
  GPIO_INPUT = (1 << 1),
  GPIO_OUTPUT = (1 << 2),

  GPIO_PP = (1 << 3),
  GPIO_OD = (1 << 4),

  GPIO_AF = (1 << 5),
};

void gpio_config(gpio_pin_def_t pin, uint32_t mode);

void gpio_set(gpio_pin_def_t pin);
void gpio_reset(gpio_pin_def_t pin);

uint8_t gpio_read(gpio_pin_def_t pin);

#endif