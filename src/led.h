#pragma once

#include <cc2510fx.h>

#include "config.h"
#include "portmacros.h"

#define BOOTLOADER_LED_GREEN_DIR PORT2DIR(BOOTLOADER_LED_GREEN_PORT)
#define BOOTLOADER_LED_RED_DIR PORT2DIR(BOOTLOADER_LED_RED_PORT)
#define BOOTLOADER_LED_RED_BIT PORT2BIT(BOOTLOADER_LED_RED_PORT, BOOTLOADER_LED_RED_PIN)
#define BOOTLOADER_LED_GREEN_BIT PORT2BIT(BOOTLOADER_LED_GREEN_PORT, BOOTLOADER_LED_GREEN_PIN)

#define led_green_init()                                         \
  {                                                              \
    BOOTLOADER_LED_GREEN_DIR |= (1 << BOOTLOADER_LED_GREEN_PIN); \
    led_green_off();                                             \
  }
#define led_green_on() \
  { BOOTLOADER_LED_GREEN_BIT = 1; }
#define led_green_off() \
  { BOOTLOADER_LED_GREEN_BIT = 0; }
#define led_green_toggle() \
  { BOOTLOADER_LED_GREEN_BIT = !BOOTLOADER_LED_GREEN_BIT; }

#define led_red_init()                                       \
  {                                                          \
    BOOTLOADER_LED_RED_DIR |= (1 << BOOTLOADER_LED_RED_PIN); \
    led_red_off();                                           \
  }
#define led_red_on() \
  { BOOTLOADER_LED_RED_BIT = 1; }
#define led_red_off() \
  { BOOTLOADER_LED_RED_BIT = 0; }
#define led_red_toggle() \
  { BOOTLOADER_LED_RED_BIT = !BOOTLOADER_LED_RED_BIT; }

// led init routines
#define led_init()    \
  {                   \
    led_red_init();   \
    led_green_init(); \
  }
