#include "delay.h"

#include "driver.h"

void delay_us(uint32_t us) {
  volatile uint32_t delay = us * (SystemCoreClock / 1000000L);
  volatile uint32_t start = DWT->CYCCNT;
  while (DWT->CYCCNT - start < delay) {
    __NOP();
  }
}

void delay_ms(uint16_t ms) {
  delay_us(ms * 1000);
}
