#include "timer.h"

#include "driver.h"

void timer_init() {
  SysTick_Config(SystemCoreClock / 10000L);

  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
}

static volatile uint32_t timeout_counter = 0;

void timer_timeout_set_100us(uint32_t us100) {
  timeout_counter = us100;
}

void timer_timeout_set_ms(uint16_t ms) {
  timeout_counter = 10 * ms;
}

uint8_t timer_timeout() {
  return timeout_counter == 0;
}

void SysTick_Handler(void) {
  if (timeout_counter != 0) {
    timeout_counter--;
  }
}