#include "gpio.h"

#include "driver.h"

#define MAKE_PIN_DEF(port, pin) gpio_pin_def_t P##port##pin = {GPIO##port, pin};
#include "gpio_pin.h.in"
#undef MAKE_PIN_DEF

void gpio_config(gpio_pin_def_t pin, uint32_t mode) {
  uint8_t mode_bit = 0;
  if ((mode & GPIO_AF) != 0) {
    mode_bit = 0b10;
  } else if ((mode & GPIO_OUTPUT) != 0) {
    mode_bit = 0b01;
  } else {
    mode_bit = 0b00;
  }
  MODIFY_REG(pin.port->MODER, (0b11 << (pin.index * 2)), mode_bit << (pin.index * 2));

  uint8_t cnf_bit = 0;
  if ((mode & GPIO_PP) != 0) {
    cnf_bit = 0b0;
  } else if ((mode & GPIO_OD) != 0) {
    cnf_bit = 0b1;
  }
  MODIFY_REG(pin.port->OTYPER, (0b1 << (pin.index)), cnf_bit << (pin.index));

  uint8_t pull_bit = 0;
  if ((mode & GPIO_PULL_UP) != 0) {
    pull_bit = 0b01;
  } else if ((mode & GPIO_PULL_DOWN) != 0) {
    pull_bit = 0b10;
  }
  MODIFY_REG(pin.port->PUPDR, (0b11 << (pin.index * 2)), pull_bit << (pin.index * 2));

  MODIFY_REG(pin.port->OSPEEDR, (0b11 << (pin.index * 2)), 0b11 << (pin.index * 2));
}

void gpio_config_af(gpio_pin_def_t pin, uint8_t af) {
  if (pin.index <= 7) {
    uint8_t offset = pin.index * 4;
    MODIFY_REG(pin.port->AFR[0], (0b1111 << offset), af << offset);
  } else {
    uint8_t offset = (pin.index - 8) * 4;
    MODIFY_REG(pin.port->AFR[1], (0b1111 << offset), af << offset);
  }
}

void gpio_set(gpio_pin_def_t pin) {
  WRITE_REG(pin.port->BSRR, (GPIO_BSRR_BS_0 << pin.index));
}

void gpio_reset(gpio_pin_def_t pin) {
  WRITE_REG(pin.port->BSRR, (GPIO_BSRR_BR_0 << pin.index));
}

uint8_t gpio_read(gpio_pin_def_t pin) {
  return READ_BIT(pin.port->IDR, (GPIO_IDR_0 << pin.index)) != 0;
}