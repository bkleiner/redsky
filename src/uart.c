#include "uart.h"

#include "delay.h"
#include "led.h"

#define BAUD_M_115200 34
#define BAUD_E_115200 12

#define UxGCR_ORDER (1 << 5)

#define UxCSR_MODE_ENABLE 0x80
#define UxCSR_TX_BYTE (1 << 1)

#define UART_TX_BUF_SIZE 128

#define HI(a) (uint8_t)((uint16_t)(a) >> 8)
#define LO(a) (uint8_t)(((uint16_t)a) & 0xFF)
#define SET_WORD(H, L, val) \
  {                         \
    (H) = HI(val);          \
    (L) = LO(val);          \
  }

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

// Define data structure for DMA descriptor:
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

__xdata dma_desc_t dma_desc;

volatile uint8_t dma_transfer_done = 1;
volatile uint8_t uart_tx_offset = 0;
volatile __xdata uint8_t uart_tx_buf[UART_TX_BUF_SIZE];

void uart_init() {
  PERCFG &= ~(PERCFG_U0CFG);
  P0SEL |= PIN_3;
  P0DIR |= PIN_3;

  U0BAUD = BAUD_M_115200;
  U0GCR = (U0GCR & ~0x1F) | (BAUD_E_115200);

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

  SET_WORD(dma_desc.SRCADDRH, dma_desc.SRCADDRL, uart_tx_buf);
  SET_WORD(dma_desc.LENH, dma_desc.LENL, UART_TX_BUF_SIZE);
  SET_WORD(dma_desc.DESTADDRH, dma_desc.DESTADDRL, 0xDFC1);

  dma_desc.VLEN = 0x2;     // Use fixed length DMA transfer count
  dma_desc.WORDSIZE = 0x0; // Perfrom 1-byte DMA transfers
  dma_desc.TMODE = 0x0;    // Single byte transfer per DMA trigger

  dma_desc.TRIG = 15;     // DMA trigger = USART0 TX complete
  dma_desc.SRCINC = 0x1;  // Increment source pointer by 1 byte address after each transfer.
  dma_desc.DESTINC = 0x0; // Do not increment destination pointer: points to USART UxDBUF register.

  dma_desc.IRQMASK = 0x1;  // Enable DMA interrupt to the CPU
  dma_desc.M8 = 0x0;       // Use all 8 bits for transfer count
  dma_desc.PRIORITY = 0x0; // DMA memory access has low priority

  SET_WORD(DMA0CFGH, DMA0CFGL, &dma_desc);

  DMAIE = 1;
  DMAIF = 0;
}

void uart_dma_isr(void) __interrupt(DMA_VECTOR) {
  DMAIF = 0;

  if (DMAIRQ & DMA_ARM_CH0) {
    DMAIRQ &= ~DMA_ARM_CH0;

    uart_tx_offset = 0;
    uart_tx_buf[0] = 0;

    dma_transfer_done = 1;
  }

  led_red_off();
}

void uart_flush() {
  while (dma_transfer_done == 0)
    ;

  dma_transfer_done = 0;
  uart_tx_buf[0] = uart_tx_offset + 1;
  led_red_on();

  // ARM DMA channel 0
  DMAARM |= DMA_ARM_CH0;
  delay_45_nop();

  DMAREQ |= DMA_ARM_CH0;
}

void uart_put(uint8_t c) {
  while (dma_transfer_done == 0)
    ;

  if (uart_tx_offset >= (UART_TX_BUF_SIZE - 1)) {
    uart_flush();
    uart_put(c);
    return;
  }

  uart_tx_buf[uart_tx_offset + 1] = c;
  uart_tx_offset++;
}

void uart_print(const char *str) {
  while (*str != 0) {
    uart_put(*str);
    str++;
  }
  uart_flush();
}