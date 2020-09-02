#include "uart.h"

#include "delay.h"
#include "dma.h"
#include "led.h"

#define BAUD_M_115200 34
#define BAUD_E_115200 12

#define BAUD_M_230400 34
#define BAUD_E_230400 13

#define UxGCR_ORDER (1 << 5)

#define UxCSR_MODE_ENABLE 0x80
#define UxCSR_TX_BYTE (1 << 1)

#define UART_TX_BUF_SIZE 16

typedef union {
  uint8_t raw;
  struct {
    uint8_t START : 1;  // start bit level
    uint8_t STOP : 1;   // stop bit level
    uint8_t SPB : 1;    // stop bits (0=1, 1=2)
    uint8_t PARITY : 1; // parity (on/off)
    uint8_t BIT9 : 1;   // 9 bit mode
    uint8_t D9 : 1;     // 9th bit level or parity type
    uint8_t FLOW : 1;   // flow control
    uint8_t ORDER : 1;  // data bit order (LSB or MSB first)
  };
} uart_config_t;

volatile uint8_t uart_dma_transfer_done = 1;
volatile uint8_t uart_tx_offset = 0;
volatile __xdata uint8_t uart_tx_buf[UART_TX_BUF_SIZE];

void uart_init() {
  PERCFG &= ~(PERCFG_U0CFG);
  P0SEL |= PIN_3;
  P0DIR |= PIN_3;

  U0BAUD = BAUD_M_115200;
  U0GCR = (U0GCR & ~0x1F) | (BAUD_E_115200);

  //U0BAUD = BAUD_M_230400;
  //U0GCR = (U0GCR & ~0x1F) | (BAUD_E_230400);

  __xdata uart_config_t config;

  config.START = 0;
  config.STOP = 1;
  config.SPB = 0;
  config.PARITY = 0;
  config.BIT9 = 0;
  config.D9 = 0;
  config.FLOW = 0;
  config.ORDER = 0;

  U0CSR |= UxCSR_MODE_ENABLE;
  U0UCR = config.raw & (0x7F);
  if (config.ORDER) {
    U0GCR |= UxGCR_ORDER;
  } else {
    U0GCR &= ~UxGCR_ORDER;
  }

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
  dma_desc[1].PRIORITY = 0x0; // DMA memory access has low priority

  SET_WORD(DMA1CFGH, DMA1CFGL, &dma_desc[1]);

  // ARM DMA channel 0
  DMAARM |= DMA_CH1;
  delay_45_nop();
}

void uart_dma_isr() {
  uart_tx_offset = 0;
  uart_tx_buf[0] = 1;

  uart_dma_transfer_done = 1;
}

void uart_flush() {
  while (uart_dma_transfer_done == 0)
    ;

  uart_dma_transfer_done = 0;
  uart_tx_buf[0] = uart_tx_offset + 1;

  // ARM DMA channel 0
  DMAARM |= DMA_CH1;
  delay_45_nop();

  DMAREQ |= DMA_CH1;
}

void uart_put(uint8_t c) {
  while (uart_dma_transfer_done == 0)
    ;

  if (uart_tx_offset >= (UART_TX_BUF_SIZE - 1)) {
    uart_flush();
    uart_put(c);
    return;
  }

  uart_tx_buf[uart_tx_offset + 1] = c;
  uart_tx_offset++;
}

void uart_update() {
  if ((DMAARM & DMA_CH1) != 0 || uart_dma_transfer_done == 0) {
    return;
  }
  DMAARM |= DMA_CH1;
}

void uart_start(uint8_t *data, uint16_t len) {
  if ((DMAARM & DMA_CH1) == 0 || uart_dma_transfer_done == 0) {
    return;
  }

  for (uint16_t i = 0; i < len; i++) {
    uart_tx_buf[i + 1] = data[i];
  }

  uart_dma_transfer_done = 0;
  uart_tx_buf[0] = len + 1;
  uart_tx_offset = len;

  DMAREQ |= DMA_CH1;
}

void uart_print(const char *str) {
  while (*str != 0) {
    uart_put(*str);
    str++;
  }
  uart_flush();
}