#include <stdint.h>

#include "cc25xx.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"
#include "flash.h"
#include "led.h"
#include "radio.h"
#include "timer.h"

int main() {
  led_init();

  led_red_on();
  clock_init_fast();
  //clock_init();
  timer_init();
  dma_init();
  led_red_off();

  EA = 1;

  led_red_on();
  uart_init();
  led_red_off();

  led_red_on();
  debug_print("booting...\r\n");
  flash_init();
  radio_init();
  led_red_off();

  delay_ms(500);

  redpine_init();
  redpine_main();
}
