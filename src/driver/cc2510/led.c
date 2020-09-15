#include "led.h"

#include "driver.h"

#define LED_GREEN_DIR PORT2DIR(LED_GREEN_PORT)
#define LED_RED_DIR PORT2DIR(LED_RED_PORT)
#define LED_RED_BIT PORT2BIT(LED_RED_PORT, LED_RED_PIN)
#define LED_GREEN_BIT PORT2BIT(LED_GREEN_PORT, LED_GREEN_PIN)

void led_green_on() {
  NOP();
  LED_GREEN_BIT = 1;
  NOP();
}

void led_green_off() {
  NOP();
  LED_GREEN_BIT = 0;
  NOP();
}

void led_green_toggle() {
  NOP();
  LED_GREEN_BIT = !LED_GREEN_BIT;
  NOP();
}

void led_red_on() {
  NOP();
  LED_RED_BIT = 1;
  NOP();
}

void led_red_off() {
  NOP();
  LED_RED_BIT = 0;
  NOP();
}

void led_red_toggle() {
  NOP();
  LED_RED_BIT = !LED_RED_BIT;
  NOP();
}

void led_green_init() {
  NOP();
  LED_GREEN_DIR |= (1 << LED_GREEN_PIN);
  NOP();
  led_green_off();
  NOP();
}

void led_red_init() {
  NOP();
  LED_RED_DIR |= (1 << LED_RED_PIN);
  NOP();
  led_red_off();
  NOP();
}

void led_init() {
  led_red_init();
  led_green_init();
}