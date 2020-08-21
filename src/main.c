#include <stdint.h>

#include "delay.h"
#include "led.h"

void main(void) {
  led_init();

  while (1) {
    delay_ms(500);
    led_green_toggle();
  }
}
