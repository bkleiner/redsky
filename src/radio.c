#include "radio.h"

#include "debug.h"
#include "dma.h"

#include "redpine.h"

#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE

volatile __xdata uint8_t packet[RADIO_RX_BUF_SIZE];
static volatile uint8_t received_packet = 0;

void radio_init() {
  debug_print("radio_init\r\n");
  IP1 |= (1 << 0);
  IP0 |= (1 << 0);

  SET_WORD(dma_desc[0].SRCADDRH, dma_desc[0].SRCADDRL, &X_RFD);
  SET_WORD(dma_desc[0].DESTADDRH, dma_desc[0].DESTADDRL, packet);
  SET_WORD(dma_desc[0].LENH, dma_desc[0].LENL, RADIO_RX_BUF_SIZE);

  dma_desc[0].VLEN = 0x04;
  dma_desc[0].WORDSIZE = 0x0;
  dma_desc[0].TMODE = 0x0;

  dma_desc[0].TRIG = 19;
  dma_desc[0].SRCINC = 0x0;
  dma_desc[0].DESTINC = 0x1;

  dma_desc[0].IRQMASK = 0x0;
  dma_desc[0].M8 = 0x0;
  dma_desc[0].PRIORITY = 0x02;

  IEN2 |= (1 << 0);
  RFIM = (1 << 4);
}

uint8_t radio_received_packet() {
  return received_packet;
}

void radio_reset_packet() {
  received_packet = 0;
}

void radio_enable_rx() {
  if ((DMAARM & DMA_CH0) == 0)
    DMAARM |= DMA_CH0;
}

void radio_dma_isr() {
}

void radio_isr(void) __interrupt(RF_VECTOR) {
  RFIF &= ~(1 << 4);
  S1CON &= ~0x03;

  radio_enable_rx();
  received_packet = 1;
}