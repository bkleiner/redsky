#include "timer.h"

#include "led.h"

#define CLOCK_TICKSPD_001 (0b00001000)
#define CLOCK_TICKSPD_010 (0b00010000)
#define CLOCK_TICKSPD_011 (0b00011000)
#define CLOCK_TICKSPD_100 (0b00100000)
#define CLOCK_TICKSPD_101 (0b00101000)
#define CLOCK_TICKSPD_110 (0b00110000)
#define CLOCK_TICKSPD_111 (0b00111000)

#define TIMER_DIV_2 (0b001 << 5)

#define TIMER_START (1 << 4)
#define TIMER_OVFIM (1 << 3)
#define TIMER_CLR (1 << 2)

#define TIMER_MODE_MODULO (0b10)

void clock_init() {
  led_red_on();
  led_green_on();

  SLEEP &= ~OSC_PD_BIT;

  while (!(SLEEP & XOSC_STABLE_BIT))
    ;

  CLKCON = 0x80;

  while (!(SLEEP & XOSC_STABLE_BIT))
    ;

  SLEEP |= OSC_PD_BIT;

  led_red_off();
  led_green_off();
}

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
  T3IE = 1;
}

static volatile uint32_t timeout_counter = 0;

void timer_isr(void) __interrupt(T3_VECTOR) {
  T3IF = 0;

  if (timeout_counter != 0) {
    timeout_counter--;
  }
}

void timer_timeout_set_us(uint32_t us) {
  T3IE = 0;
  timeout_counter = us / 40;
  T3IE = 1;
}

void timer_timeout_set_ms(uint32_t ms) {
  timer_timeout_set_us(ms * 1000);
}

uint8_t timer_timeout() {
  return timeout_counter == 0;
}