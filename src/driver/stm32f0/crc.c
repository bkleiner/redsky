#include "crc.h"

#include "driver.h"

#define REDPINE_CRC16_POLY 0x8005

uint16_t crc_compute(uint8_t *data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 3; i < len; i++) {
    uint8_t val = data[i];

    for (uint8_t i = 0; i < 8; i++) {
      if (((crc & 0x8000) >> 8) ^ (val & 0x80))
        crc = (crc << 1) ^ REDPINE_CRC16_POLY;
      else
        crc = (crc << 1);
      val <<= 1;
    }
  }
  return crc;
}