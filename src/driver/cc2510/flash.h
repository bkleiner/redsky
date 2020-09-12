#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

#include "driver.h"

void flash_init();
void flash_read(uint16_t addr, uint8_t *buf, uint16_t len);
void flash_write(uint16_t addr, uint8_t *buf, uint16_t len);
void flash_erase(uint16_t page);

#endif