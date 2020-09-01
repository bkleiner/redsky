#include <stdint.h>

#include "cc25xx.h"

#include "delay.h"
#include "dma.h"
#include "led.h"
#include "radio.h"
#include "timer.h"
#include "uart.h"

void enable_inverter() {
  P1DIR |= (1 << 0);
  P1_0 = 1;
}

int main() {
  led_init();
  clock_init();

  EA = 1;
  timer_init();
  dma_init();

  led_red_on();
  uart_init();
  enable_inverter();
  led_red_off();

  led_red_on();
  radio_init();
  led_red_off();

  delay_ms(100);

  uart_print("booting...\r\n");

  while (1) {
    if (timer_timeout()) {
      led_green_toggle();

      uart_put('0');
      uart_flush();
      timer_timeout_set_ms(100);
    }
  }
}
