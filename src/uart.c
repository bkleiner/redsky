#include "uart.h"

#include "cc25xx.h"
#include "led.h"

#define BAUD_M_115200 34
#define BAUD_E_115200 12

#define UxGCR_ORDER (1 << 5)

#define UxCSR_MODE_ENABLE 0x80
#define UxCSR_TX_BYTE (1 << 1)

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
}

void uart_put(uint8_t c) {
  UTX0IF = 0;
  U0DBUF = c;

  led_red_toggle();

  while (!UTX0IF)
    ;
  UTX0IF = 0;
}

void uart_print(const char *str) {
  while (*str != 0) {
    uart_put(*str);
    str++;
  }
}