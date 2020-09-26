#include "clock.h"
#include "delay.h"
#include "led.h"
#include "uart.h"

int main() {
  led_init();

  //clock_init();
  uart_init();

  while (1) {
    led_red_toggle();
    uart_put('!');
    delay_ms(500);
    led_green_toggle();
  }
}