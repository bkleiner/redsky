#ifndef DMA_H
#define DMA_H

#include <stdint.h>

#include "cc25xx.h"

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

extern __xdata dma_desc_t dma_desc[5];

void dma_init();
void dma_isr(void) __interrupt(DMA_VECTOR);

#endif