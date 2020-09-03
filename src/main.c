#include <stdint.h>

#include "cc25xx.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"
#include "led.h"
#include "radio.h"
#include "timer.h"

int main() {
  led_init();
  clock_init();

  EA = 1;
  timer_init();
  dma_init();

  led_red_on();
  uart_init();
  led_red_off();

  led_red_on();
  radio_init();
  led_red_off();

  delay_ms(100);
  debug_print("booting...\r\n");

  redpine_init();
  redpine_main();
}
