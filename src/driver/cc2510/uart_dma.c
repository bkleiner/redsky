#include "uart_dma.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"
#include "led.h"
#include "uart.h"

#include <string.h>

#define UART_TX_BUF_SIZE 64

static volatile uint8_t uart_dma_transfer_done;
static volatile __xdata uint8_t uart_tx_buf[UART_TX_BUF_SIZE];

#define uart_dma_armed ((DMAARM & DMA_CH1) != 0 ? 1 : 0)

void uart_dma_init() {
  uart_init();

  WRITE_WORD(dma_desc[1].SRCADDRH, dma_desc[1].SRCADDRL, uart_tx_buf);
  WRITE_WORD(dma_desc[1].LENH, dma_desc[1].LENL, UART_TX_BUF_SIZE);
  WRITE_WORD(dma_desc[1].DESTADDRH, dma_desc[1].DESTADDRL, 0xDFC1);

  dma_desc[1].VLEN = 0x0;
  dma_desc[1].WORDSIZE = 0x0;
  dma_desc[1].TMODE = 0x0;

  dma_desc[1].TRIG = 15;
  dma_desc[1].SRCINC = 0x1;
  dma_desc[1].DESTINC = 0x0;

  dma_desc[1].IRQMASK = 0x1;
  dma_desc[1].M8 = 0x0;
  dma_desc[1].PRIORITY = 0x1;

  uart_dma_transfer_done = 1;
}

void uart_dma_isr() {
  while (!(U0CSR & UxCSR_TX_BYTE))
    ;
  U0CSR &= ~UxCSR_TX_BYTE;
  uart_dma_transfer_done = 1;
}

inline uint8_t uart_update(uint16_t len) {
  if (uart_dma_transfer_done == 0) {
    return 0;
  }
  if (uart_dma_armed == 1) {
    return 1;
  }

  WRITE_WORD(dma_desc[1].LENH, dma_desc[1].LENL, len);

  DMAARM |= DMA_CH1;
  delay_us(15);
  return 1;
}

inline uint8_t uart_flush() {
  uart_dma_transfer_done = 0;
  DMAIRQ &= ~DMA_CH1;
  DMAREQ |= DMA_CH1;
  return 1;
}

uint8_t uart_dma_start(uint8_t *data, uint16_t len) {
  if (uart_update(len) == 0) {
    return 0;
  }

  memcpy(uart_tx_buf, data, len);

  return uart_flush();
}

uint16_t _strlen(const char *str) {
  char *ptr = (char *)str;
  while (*ptr) {
    ptr++;
  }
  return (ptr - str);
}

void uart_dma_print(const char *str) {
  const uint16_t len = _strlen(str);

  while (uart_update(len) == 0)
    ;

  uart_dma_start((uint8_t *)str, len);
}

#ifdef DEBUG_OUTPUT
void uart_dma_printf(char *fmt, ...) {
  while (uart_dma_transfer_done == 0)
    ;

  va_list va;
  va_start(va, fmt);
  uint16_t len = debug_vsnprintf(uart_tx_buf, UART_TX_BUF_SIZE, fmt, va);
  va_end(va);

  while (uart_update(len) == 0)
    ;

  uart_flush();
}
#endif