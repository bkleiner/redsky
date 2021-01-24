#include "delay.h"

#include "driver.h"

void delay_us(uint32_t us) {
  volatile uint32_t count = us * 5;
  while (count--)
    ;
}

void delay_ms(uint16_t ms) {
  delay_us(ms * 1000);
}
