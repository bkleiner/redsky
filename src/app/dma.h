#ifndef DMA_H
#define DMA_H

#include <stdint.h>

#include "cc25xx.h"

extern __xdata dma_desc_t dma_desc[5];

void dma_init();
void dma_isr(void) __interrupt(DMA_VECTOR);

#endif