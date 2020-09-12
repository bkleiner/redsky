#include "gpio.h"

#include "driver.h"

void gpio_config(GPIO_TypeDef *port, uint8_t pin, uint32_t mode) {
  uint8_t cnf_bit = 0;
  uint8_t mode_bit = 0;
  if ((mode & GPIO_OUTPUT) != 0) {
    mode_bit = 0b11;

    if ((mode & GPIO_OD) != 0) {
      cnf_bit |= 0b01;
    }

    if ((mode & GPIO_AF) != 0) {
      cnf_bit |= 0b10;
    }
  } else {
    if ((mode & GPIO_OD) != 0) {
      cnf_bit |= 0b01;
    }
  }

  if (pin <= 7) {
    uint8_t offset = pin * 4;
    MODIFY_REG(port->CRL, (0b1111 << offset), ((cnf_bit << 2) | mode_bit) << offset);
  } else {
    uint8_t offset = (pin - 8) * 4;
    MODIFY_REG(port->CRH, (0b1111 << offset), ((cnf_bit << 2) | mode_bit) << offset);
  }
}

void gpio_set(GPIO_TypeDef *port, uint8_t pin) {
  WRITE_REG(port->BSRR, (GPIO_BSRR_BS0 << pin));
}

void gpio_reset(GPIO_TypeDef *port, uint8_t pin) {
  WRITE_REG(port->BSRR, (GPIO_BSRR_BR0 << pin));
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin) {
  return READ_BIT(port->ODR, (GPIO_ODR_ODR0 << pin));
}