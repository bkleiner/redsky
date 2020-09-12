#include "uart.h"

#include "driver.h"

#include "delay.h"

#define BAUD_M_115200 34
#define BAUD_E_115200 12

#define BAUD_M_230400 34
#define BAUD_E_230400 13

#define UxGCR_ORDER (1 << 5)

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

inline void enable_inverter() {
  if (P1_0 != 1) {
    P1_0 = 1;
  }
}

inline void disable_inverter() {
  if (P1_0 != 0) {
    P1_0 = 0;
  }
}

void uart_init() {
  // enable inverter pin
  P1DIR |= (1 << 0);
  enable_inverter();

  PERCFG &= ~(PERCFG_U0CFG);
  P0SEL |= PIN_3;
  P0SEL |= PIN_2;

  //U0BAUD = BAUD_M_115200;
  //U0GCR = (U0GCR & ~0x1F) | (BAUD_E_115200);

  U0BAUD = BAUD_M_230400;
  U0GCR = (U0GCR & ~0x1F) | (BAUD_E_230400);

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
  U0CSR |= UxCSR_RX_ENABLE;
}

uint8_t uart_get_no_timeout() {
  disable_inverter();

  URX0IF = 0;
  while (!URX0IF)
    ;
  uint8_t val = U0DBUF;
  URX0IF = 0;

  return val;
}

void uart_put(uint8_t val) {
  enable_inverter();

  UTX0IF = 0;
  U0DBUF = val;
  while (!UTX0IF || !(U0CSR & UxCSR_TX_BYTE))
    ;
  U0CSR &= ~UxCSR_TX_BYTE;
  UTX0IF = 0;
}

uint8_t uart_get(uint8_t *val, uint16_t timeout) {
  disable_inverter();

  URX0IF = 0;

  while (!URX0IF && --timeout > 0)
    ;

  if (timeout == 0) {
    return 0;
  }

  *val = U0DBUF;
  URX0IF = 0;

  return 1;
}