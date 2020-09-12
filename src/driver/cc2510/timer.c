#include "timer.h"

#include "led.h"

#define CLOCK_TICKSPD_001 ((0b001) << 3)
#define CLOCK_TICKSPD_010 ((0b010) << 3)
#define CLOCK_TICKSPD_011 ((0b011) << 3)
#define CLOCK_TICKSPD_100 ((0b100) << 3)
#define CLOCK_TICKSPD_101 ((0b101) << 3)
#define CLOCK_TICKSPD_110 ((0b110) << 3)
#define CLOCK_TICKSPD_111 ((0b111) << 3)

#define TIMER_DIV_2 (0b001 << 5)
#define TIMER_DIV_4 (0b010 << 5)

#define TIMER_START (1 << 4)
#define TIMER_OVFIM (1 << 3)
#define TIMER_CLR (1 << 2)

#define TIMER_MODE_MODULO (0b10)

void timer_init() {
  CLKCON = (CLKCON & ~CLOCK_TICKSPD_111) | CLOCK_TICKSPD_011;

  // TICKSPD 011 -> /8 = 3250 kHz timer clock input
  T3CTL = TIMER_DIV_2 |
          TIMER_START |
          TIMER_OVFIM |
          TIMER_CLR |
          TIMER_MODE_MODULO;

  // 3250/2/65 = 25khz = 40us steps
  T3CC0 = 65 - 1;
}

static volatile uint16_t timeout_counter = 0;

void timer3_isr(void) __interrupt(T3_VECTOR) {
  T3IF = 0;

  if (timeout_counter != 0) {
    timeout_counter--;
  }
}

void timer_timeout_set_100us(uint16_t us100) {
  T3IE = 0;
  timeout_counter = (us100 * 25) / 10;
  T3IE = 1;
}

void timer_timeout_set_ms(uint16_t ms) {
  T3IE = 0;
  timeout_counter = (ms * 25);
  T3IE = 1;
}

uint8_t timer_timeout() __critical {
  if (timeout_counter == 0) {
    return 1;
  }
  return 0;
}