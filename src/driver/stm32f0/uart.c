#include "uart.h"

#include "driver.h"
#include "gpio.h"

void uart_init() {
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

  gpio_config(UART_TX, GPIO_OUTPUT | GPIO_PP | GPIO_AF);
  gpio_config_af(UART_TX, 1);

  // rx pin, not actually used
  gpio_config(UART_RX, GPIO_INPUT | GPIO_OD);
  gpio_config_af(UART_RX, 1);

  // half duplex
  //USART1->CR3 = USART_CR3_HDSEL;
  USART1->CR2 |= USART_CR2_TXINV | USART_CR2_RXINV;

#ifdef SERIAL_SBUS
  // 1 parity bit
  USART1->CR1 |= USART_CR1_M | USART_CR1_PCE;
  // 2 stop bits
  USART1->CR2 |= USART_CR2_STOP_1;
  USART1->BRR = (SystemCoreClock / 200000);
#endif

#ifdef SERIAL_REDPINE
  USART1->BRR = (SystemCoreClock / 230400);
#endif

  // enable uart, enable transmission
  USART1->CR1 |= USART_CR1_UE | USART_CR1_TE;
}

void uart_put(uint8_t val) {
  USART1->CR1 &= ~USART_CR1_RE;
  USART1->CR1 |= USART_CR1_TE;

  while (!(USART1->ISR & USART_ISR_TXE))
    ;
  USART1->TDR = val;
  while (!(USART1->ISR & USART_ISR_TC))
    ;
}

uint8_t uart_get(uint8_t *val, uint16_t timeout) {
  USART1->CR1 &= ~USART_CR1_TE;
  USART1->CR1 |= USART_CR1_RE;

  while (!(USART1->ISR & USART_ISR_RXNE) && --timeout > 0)
    ;

  if (timeout == 0) {
    return 0;
  }

  *val = USART1->RDR;
  return 1;
}