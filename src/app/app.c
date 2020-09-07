#include "app.h"

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

  delay_ms(250);
  debug_print("booting...\r\n");

  led_red_on();
  flash_init();
  radio_init();
  led_red_off();

  delay_ms(250);

  redpine_init();
  redpine_main();
}
