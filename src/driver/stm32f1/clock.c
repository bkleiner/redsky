#include "clock.h"

#include "driver.h"
#include "led.h"

extern void SystemCoreClockUpdate();

void clock_init() {
  SystemCoreClockUpdate();

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
