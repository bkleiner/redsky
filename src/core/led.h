#ifndef LED_H
#define LED_H

#include "cc25xx.h"

void led_init();

void led_green_on();
void led_green_off();
void led_green_toggle();

void led_red_on();
void led_red_off();
void led_red_toggle();

#endif