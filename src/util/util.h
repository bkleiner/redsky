#ifndef UTIL_H
#define UTIL_H

#define HI(a) (uint8_t)((uint16_t)(a) >> 8)
#define LO(a) (uint8_t)(((uint16_t)a) & 0xFF)

#define WRITE_WORD(H, L, val) \
  {                           \
    (H) = HI(val);            \
    (L) = LO(val);            \
  }

#define READ_WORD(H, L) ((H << 8) | L)

#endif