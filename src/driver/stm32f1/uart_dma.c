#include "uart_dma.h"

#include <string.h>

void uart_dma_init() {
  uart_init();

  SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
}

static uint8_t dma_buf[128];

uint8_t uart_dma_start(uint8_t *data, uint16_t len) {
  memcpy(dma_buf, data, len);

  DMA1_Channel4->CCR |= DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_DIR;
  DMA1_Channel4->CPAR = (uint32_t)(&USART1->DR);
  DMA1_Channel4->CMAR = (uint32_t)(&dma_buf);
  DMA1_Channel4->CNDTR = len;
  DMA1_Channel4->CCR |= DMA_CCR_EN;

  USART1->CR3 |= USART_CR3_DMAT;
  return 1;
}

uint16_t _strlen(const char *str) {
  char *ptr = (char *)str;
  while (*ptr) {
    ptr++;
  }
  return (ptr - str);
}

void uart_dma_print(const char *str) {
  uart_dma_start((uint8_t *)str, _strlen(str));

  while (!(DMA1->ISR & DMA_ISR_TCIF4))
    ;

  DMA1->IFCR |= DMA_ISR_TCIF4;

  USART1->CR3 &= ~USART_CR3_DMAT;
  DMA1_Channel4->CCR &= ~DMA_CCR_EN;
}