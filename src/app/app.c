#include "app.h"

int main() {
  led_init();

  led_red_on();
  clock_init();
  timer_init();
  dma_init();
  led_red_off();

  led_red_on();
  uart_dma_init();
  led_red_off();

  delay_ms(150);
  debug_print("booting...\r\n");

  led_red_on();
  storage_init();
  radio_init();
  led_red_off();

  delay_ms(250);

  redpine_init();
  redpine_main();

  return 1;
}
