#include "dma.h"

#include "radio.h"
#include "uart.h"

__xdata dma_desc_t dma_desc[5];

void dma_init() {
  DMAIE = 1;
  DMAIF = 0;
}

void dma_isr(void) __interrupt(DMA_VECTOR) {
  DMAIF = 0;

  if (DMAIRQ & DMA_CH0) {
    DMAIRQ &= ~DMA_CH0;

    radio_dma_isr();
  }
  if (DMAIRQ & DMA_CH1) {
    DMAIRQ &= ~DMA_CH1;

    uart_dma_isr();
  }
}