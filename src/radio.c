#include "radio.h"

#include "dma.h"

#define RADIO_RX_BUF_SIZE 128

volatile __xdata uint8_t radio_rx_buf[RADIO_RX_BUF_SIZE];

void radio_init() {
  SET_WORD(dma_desc[0].SRCADDRH, dma_desc[0].SRCADDRL, &X_RFD);
  SET_WORD(dma_desc[0].LENH, dma_desc[0].LENL, RADIO_RX_BUF_SIZE);
  SET_WORD(dma_desc[0].DESTADDRH, dma_desc[0].DESTADDRL, 0xDFC1);

  dma_desc[0].VLEN = 0x0;
  dma_desc[0].WORDSIZE = 0x0;
  dma_desc[0].TMODE = 0x0;

  dma_desc[0].TRIG = 19;
  dma_desc[0].SRCINC = 0x0;
  dma_desc[0].DESTINC = 0x1;

  dma_desc[0].IRQMASK = 0x1;
  dma_desc[0].M8 = 0x0;
  dma_desc[0].PRIORITY = 0x0;

  SET_WORD(DMA0CFGH, DMA0CFGL, &dma_desc[0]);
}

void radio_dma_isr() {
}

void radio_isr(void) __interrupt(RF_VECTOR) {
}