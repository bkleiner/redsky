#ifndef DRIVER_H
#define DRIVER_H

#include <stm32f1xx.h>

#include "config.h"

#define EXT_MEMORY

extern uint32_t SystemCoreClock;

#define HI(a) (uint8_t)((uint16_t)(a) >> 8)
#define LO(a) (uint8_t)(((uint16_t)a) & 0xFF)
#define SET_WORD(H, L, val) \
  {                         \
    (H) = HI(val);          \
    (L) = LO(val);          \
  }

#endif