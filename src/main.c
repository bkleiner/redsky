#include <stdint.h>

#include "cc25xx.h"

#include "delay.h"
#include "led.h"
#include "uart.h"

void init_clock() {
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

void enable_inverter() {
  P1DIR |= (1 << 0);
  P1_0 = 1;
}

int main() {
  led_init();
  init_clock();

  EA = 1;

  led_red_on();
  uart_init();
  enable_inverter();
  led_red_off();

  delay_ms(100);

  uart_print("booting...\r\n");

  while (1) {
    delay_ms(500);
    led_green_toggle();

    uart_put('0');
  }
}
