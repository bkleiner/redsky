#include "crc.h"

#include "driver.h"

uint16_t crc_compute(uint8_t *data, uint16_t len) {
  // seed crc
  RNDL = 0xFF;
  RNDL = 0xFF;

  for (uint8_t i = 3; i < len; i++) {
    RNDH = data[i];
  }

  return (RNDH << 8) | RNDL;
}