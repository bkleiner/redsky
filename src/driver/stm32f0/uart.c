#include "uart.h"

#include "driver.h"
#include "gpio.h"

void uart_init() {
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);

  gpio_config(UART_TX, GPIO_OUTPUT | GPIO_PP | GPIO_AF);
  gpio_config(UART_RX, GPIO_INPUT | GPIO_OD);

  USART1->CR1 = USART_CR1_UE + USART_CR1_TE + USART_CR1_RE;
  USART1->BRR = (SystemCoreClock / 230400);
}

void uart_put(uint8_t val) {
  while (!(USART1->ISR & USART_ISR_TXE))
    ;
  USART1->TDR = val;
  while (!(USART1->ISR & USART_ISR_TC))
    ;
}

uint8_t uart_get(uint8_t *val, uint16_t timeout) {
  while (!(USART1->ISR & USART_ISR_RXNE) && --timeout > 0)
    ;

  if (timeout == 0) {
    return 0;
  }

  *val = USART1->RDR;
  return 1;
}