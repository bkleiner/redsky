#include "clock.h"

#include "driver.h"
#include "led.h"

void clock_init() {
  led_red_on();
  led_green_on();

  SLEEP &= ~OSC_PD_BIT;

  while (!(SLEEP & XOSC_STABLE_BIT))
    ;

  CLKCON = (CLKCON & ~(0x40 | 0x7));

  while (!(SLEEP & XOSC_STABLE_BIT))
    ;

  SLEEP |= OSC_PD_BIT;

  led_red_off();
  led_green_off();

  EA = 1;
}
