#include "dma.h"

#include "radio.h"
#include "uart.h"

__xdata dma_desc_t dma_desc[5];

void dma_init() {
  SET_WORD(DMA0CFGH, DMA0CFGL, &dma_desc[0]);
  SET_WORD(DMA1CFGH, DMA1CFGL, &dma_desc[1]);

  DMAIE = 1;
  DMAIF = 0;
}

void dma_isr(void) __interrupt(DMA_VECTOR) {
  DMAIF = 0;

  if (DMAIRQ & DMA_CH0) {
    radio_dma_isr();
    DMAIRQ &= ~DMA_CH0;
  }
  if (DMAIRQ & DMA_CH1) {
    uart_dma_isr();
    DMAIRQ &= ~DMA_CH1;
  }
}