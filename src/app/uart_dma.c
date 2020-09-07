#include "uart_dma.h"

#include "delay.h"
#include "dma.h"
#include "led.h"
#include "uart.h"

#define UART_TX_BUF_SIZE 64

static volatile uint8_t uart_dma_transfer_done;
#define uart_dma_armed ((DMAARM & DMA_CH1) != 0 ? 1 : 0)
static volatile __xdata uint8_t uart_tx_buf[UART_TX_BUF_SIZE];

void uart_dma_init() {
  uart_init();

  SET_WORD(dma_desc[1].SRCADDRH, dma_desc[1].SRCADDRL, uart_tx_buf);
  SET_WORD(dma_desc[1].LENH, dma_desc[1].LENL, UART_TX_BUF_SIZE);
  SET_WORD(dma_desc[1].DESTADDRH, dma_desc[1].DESTADDRL, 0xDFC1);

  dma_desc[1].VLEN = 0x2;     // Use fixed length DMA transfer count
  dma_desc[1].WORDSIZE = 0x0; // Perfrom 1-byte DMA transfers
  dma_desc[1].TMODE = 0x0;    // Single byte transfer per DMA trigger

  dma_desc[1].TRIG = 15;     // DMA trigger = USART0 TX complete
  dma_desc[1].SRCINC = 0x1;  // Increment source pointer by 1 byte address after each transfer.
  dma_desc[1].DESTINC = 0x0; // Do not increment destination pointer: points to USART UxDBUF register.

  dma_desc[1].IRQMASK = 0x1;  // Enable DMA interrupt to the CPU
  dma_desc[1].M8 = 0x0;       // Use all 8 bits for transfer count
  dma_desc[1].PRIORITY = 0x1; // DMA memory access has low priority

  uart_tx_buf[0] = 1;
  uart_dma_transfer_done = 1;

  // ARM DMA channel 0
  DMAARM |= DMA_CH1;
  delay_45_nop();
}

void uart_dma_isr() {
  uart_dma_transfer_done = 1;
}

inline uint8_t uart_update() {
  if (uart_dma_transfer_done == 0) {
    return 0;
  }
  if (uart_dma_armed == 1) {
    return 1;
  }

  DMAARM |= DMA_CH1;
  delay_45_nop();
  return 1;
}

inline uint8_t uart_flush() {
  uart_dma_transfer_done = 0;
  DMAIRQ &= ~DMA_CH1;
  DMAREQ |= DMA_CH1;
  return 1;
}

uint8_t uart_dma_start(uint8_t *data, uint16_t len) {
  if (uart_update() == 0) {
    return 0;
  }

  for (uint16_t i = 0; i < len; i++) {
    uart_tx_buf[i + 1] = data[i];
  }
  uart_tx_buf[0] = len + 1;

  return uart_flush();
}

uint16_t _strlen(const char *str) {
  char *ptr = str;
  while (*ptr) {
    ptr++;
  }
  return (ptr - str);
}

void uart_dma_print(const char *str) {
  while (uart_update() == 0)
    ;

  uart_dma_start(str, _strlen(str));
}

#ifdef DEBUG_OUTPUT
void uart_dma_printf(char *fmt, ...) {
  while (uart_update() == 0)
    ;

  va_list va;
  va_start(va, fmt);
  static __xdata uint16_t buf_len = UART_TX_BUF_SIZE - 1;
  uint16_t len = debug_vsnprintf(&uart_tx_buf[1], buf_len, fmt, va);
  va_end(va);

  uart_tx_buf[0] = len + 1;

  uart_flush();
}
#endif