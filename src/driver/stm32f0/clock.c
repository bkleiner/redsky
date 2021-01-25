#include "clock.h"

#include "driver.h"
#include "led.h"

extern void SystemCoreClockUpdate();

void clock_init() {
  SystemCoreClockUpdate();

  SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOFEN);
  SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGCOMPEN);
}