#include "clock.h"

#include "driver.h"
#include "led.h"

extern void SystemCoreClockUpdate();

void clock_init() {
  SystemCoreClockUpdate();

  SET_BIT(RCC->AHBENR, RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOFEN);

  /*
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE);

  SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
  */
}