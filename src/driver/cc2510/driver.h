#ifndef DRIVER_H
#define DRIVER_H

#include <cc2510fx.h>
#include <stdint.h>

#include "config.h"
#include "portmacros.h"

#define EXT_MEMORY __xdata

#define PIN_0 (1 << 0)
#define PIN_1 (1 << 1)
#define PIN_2 (1 << 2)
#define PIN_3 (1 << 3)
#define PIN_4 (1 << 4)
#define PIN_5 (1 << 5)
#define PIN_6 (1 << 6)
#define PIN_7 (1 << 7)

#define PERCFG_U0CFG (1 << 0)
#define PERCFG_U1CFG (1 << 1)
#define PERCFG_T4CFG (1 << 4)
#define PERCFG_T3CFG (1 << 5)
#define PERCFG_T1CFG (1 << 6)

#define OSC_PD_BIT 0x04
#define XOSC_STABLE_BIT 0x40

#define DMA_CH0 (1 << 0)
#define DMA_CH1 (1 << 1)
#define DMA_CH2 (1 << 2)
#define DMA_CH3 (1 << 3)
#define DMA_CH4 (1 << 4)

#define DMA_ABORT (1 << 7)

#define FCTL_BUSY (1 << 7)
#define FCTL_SWBUSY (1 << 6)
#define FCTL_WRITE (1 << 1)
#define FCTL_ERASE (1 << 0)

#define UxCSR_TX_BYTE 0x2
#define UxCSR_RX_ENABLE 0x40
#define UxCSR_MODE_ENABLE 0x80

#define RFST_SNOP 0x05
#define RFST_SIDLE 0x04
#define RFST_STX 0x03
#define RFST_SRX 0x02
#define RFST_SCAL 0x01
#define RFST_SFSTXON 0x00

SFRX(TEST0, 0xDF25);
SFRX(TEST1, 0xDF24);
SFRX(TEST2, 0xDF23);

#define HI(a) (uint8_t)((uint16_t)(a) >> 8)
#define LO(a) (uint8_t)(((uint16_t)a) & 0xFF)
#define SET_WORD(H, L, val) \
  {                         \
    (H) = HI(val);          \
    (L) = LO(val);          \
  }

typedef struct {
  uint8_t SRCADDRH;  // High byte of the source address
  uint8_t SRCADDRL;  // Low byte of the source address
  uint8_t DESTADDRH; // High byte of the destination address
  uint8_t DESTADDRL; // Low byte of the destination address

  uint8_t LENH : 5; // High byte of fixed length
  uint8_t VLEN : 3; // Length configuration

  uint8_t LENL : 8; // Low byte of fixed length

  uint8_t TRIG : 5;     // DMA trigger; UART RX/TX
  uint8_t TMODE : 2;    // DMA trigger mode (e.g. single or repeated)
  uint8_t WORDSIZE : 1; // Number of bytes per transfer element

  uint8_t PRIORITY : 2; // The DMA memory access priority
  uint8_t M8 : 1;       // Number of desired bit transfers in byte mode
  uint8_t IRQMASK : 1;  // DMA interrupt mask
  uint8_t DESTINC : 2;  // Number of destination address increments
  uint8_t SRCINC : 2;   // Number of source address increments
} dma_desc_t;

#endif