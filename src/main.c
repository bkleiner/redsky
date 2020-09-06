#include <stdint.h>

#include "cc25xx.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"
#include "flash.h"
#include "led.h"
#include "radio.h"
#include "redpine.h"
#include "timer.h"

void main() {
  led_init();

  led_red_on();
  clock_init_fast();
  timer_init();
  dma_init();
  led_red_off();

  EA = 1;

  led_red_on();
  uart_init();
  led_red_off();

  debug_print("booting...\r\n");
  delay_ms(250);

  led_red_on();
  flash_init();
  radio_init();
  led_red_off();

  delay_ms(250);

  redpine_init();
  redpine_main();
}
