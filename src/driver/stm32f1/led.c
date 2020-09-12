#include "led.h"

#include "gpio.h"

#define GPIO_PIN(VAR, PIN) GPIO_PIN_(VAR, PIN)
#define GPIO_PIN_(VAR, PIN) VAR##PIN

void led_green_on() {
  gpio_set(LED_GREEN_PORT, LED_GREEN_PIN);
}

void led_green_off() {
  gpio_reset(LED_GREEN_PORT, LED_GREEN_PIN);
}

void led_green_toggle() {
  if (gpio_read(LED_GREEN_PORT, LED_GREEN_PIN)) {
    led_green_off();
  } else {
    led_green_on();
  }
}

void led_green_init() {
  gpio_config(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_OUTPUT | GPIO_PP);
  led_green_off();
}

void led_red_on() {
  gpio_set(LED_RED_PORT, LED_RED_PIN);
}

void led_red_off() {
  gpio_reset(LED_RED_PORT, LED_RED_PIN);
}

void led_red_toggle() {
  if (gpio_read(LED_RED_PORT, LED_RED_PIN)) {
    led_red_off();
  } else {
    led_red_on();
  }
}

void led_red_init() {
  gpio_config(LED_RED_PORT, LED_RED_PIN, GPIO_OUTPUT | GPIO_PP);
  led_red_off();
}

void led_init() {
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_IOPAEN + RCC_APB2ENR_AFIOEN);

  led_red_init();
  led_green_init();
}