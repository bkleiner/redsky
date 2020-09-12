#include "clock.h"
#include "delay.h"
#include "led.h"
#include "radio.h"
#include "uart_dma.h"

int main() {
  led_init();

  clock_init();
  uart_dma_init();
  radio_init();

  while (1) {
    led_red_toggle();
    uart_dma_print("test!\r\n");
    delay_ms(500);
  }
}