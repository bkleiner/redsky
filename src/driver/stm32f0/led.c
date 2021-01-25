#include "led.h"

#include "gpio.h"

void led_green_on() {
#if defined(LED_GREEN_PIN)
  gpio_set(LED_GREEN_PIN);
#endif
}

void led_green_off() {
#if defined(LED_GREEN_PIN)
  gpio_reset(LED_GREEN_PIN);
#endif
}

void led_green_toggle() {
#if defined(LED_GREEN_PIN)
  if (READ_BIT(LED_GREEN_PIN.port->ODR, (GPIO_ODR_0 << LED_GREEN_PIN.index))) {
    led_green_off();
  } else {
    led_green_on();
  }
#endif
}

void led_green_init() {
#if defined(LED_GREEN_PIN)
  gpio_config(LED_GREEN_PIN, GPIO_OUTPUT | GPIO_PP);
  led_green_off();
#endif
}

void led_red_on() {
#if defined(LED_RED_PIN)
#if defined(LED_RED_INVERT)
  gpio_reset(LED_RED_PIN);
#else
  gpio_set(LED_RED_PIN);
#endif
#endif
}

void led_red_off() {
#if defined(LED_RED_PIN)
#if defined(LED_RED_INVERT)
  gpio_set(LED_RED_PIN);
#else
  gpio_reset(LED_RED_PIN);
#endif
#endif
}

void led_red_toggle() {
#if defined(LED_RED_PIN)
  if (READ_BIT(LED_RED_PIN.port->ODR, (GPIO_ODR_0 << LED_RED_PIN.index))) {
    led_red_off();
  } else {
    led_red_on();
  }
#endif
}

void led_red_init() {
#if defined(LED_RED_PIN)
  gpio_config(LED_RED_PIN, GPIO_OUTPUT | GPIO_PP);
  led_red_off();
#endif
}

void led_init() {
  led_red_init();
  led_green_init();
}