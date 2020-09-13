#ifndef CRC_H
#define CRC_H

#include <stdint.h>

uint16_t crc_compute(uint8_t *data, uint16_t len);

#endif