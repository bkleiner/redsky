#include "clock.h"
#include "delay.h"
#include "flash.h"
#include "led.h"
#include "uart.h"

static __xdata uint8_t buffer[1024];

int main() {
  led_init();

  clock_init();
  uart_init();

  while (1) {
    led_red_toggle();
    uint8_t magic = 0;
    if (uart_get(&magic, 0x1FFF) && magic == '!') {
      for (uint8_t i = 0; i < 16; i++) {
        flash_read(i * 1024, buffer, 1024);
        uart_write(buffer, 1024);
      }
    }
  }
}