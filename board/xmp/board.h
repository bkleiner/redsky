#ifndef BOARD_H
#define BOARD_H

#define FLASH_PAGE_SIZE 1024
#define FLASH_PAGE_OFFSET 0x400

#define LED_GREEN_PORT P1
#define LED_GREEN_PIN 3
#define LED_RED_PORT P1
#define LED_RED_PIN 2

#define RF_LNA_PORT P2
#define RF_LNA_PIN 4
#define RF_LNA_ON_LEVEL 0

#define RF_ANTENNA_SWITCH_PORT P0
#define RF_ANTENNA_SWITCH_PIN 6
#define RF_ANTENNA_A_LEVEL 1

#define RF_ANTENNA_SWITCH_PORT2 P0
#define RF_ANTENNA_SWITCH_PIN2 7
#define RF_ANTENNA_A_LEVEL2 1

#endif