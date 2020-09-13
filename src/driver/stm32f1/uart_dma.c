#include "uart_dma.h"

#include <string.h>

void uart_dma_init() {
  uart_init();

  NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  DMA1->IFCR |= DMA_ISR_TCIF4;
}

static uint8_t dma_buf[128];

static volatile uint8_t dma_transfer_done = 1;

uint8_t uart_dma_start(uint8_t *data, uint16_t len) {
  if (!dma_transfer_done) {
    return 0;
  }

  dma_transfer_done = 0;

  dma_buf[0] = len + 1;
  memcpy(dma_buf + 1, data, len);

  DMA1_Channel4->CCR |= DMA_CCR_PL_0 | DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
  DMA1_Channel4->CPAR = (uint32_t)(&USART1->DR);
  DMA1_Channel4->CMAR = (uint32_t)(&dma_buf);
  DMA1_Channel4->CNDTR = len + 1;
  DMA1_Channel4->CCR |= DMA_CCR_EN;

  USART1->CR3 |= USART_CR3_DMAT;
  return 1;
}

void DMA1_Channel4_IRQHandler() {
  if (DMA1->ISR & DMA_ISR_TCIF4) {
    DMA1->IFCR |= DMA_ISR_TCIF4;

    USART1->CR3 &= ~USART_CR3_DMAT;
    DMA1_Channel4->CCR &= ~DMA_CCR_EN;

    dma_transfer_done = 1;
  }
}

uint16_t _strlen(const char *str) {
  char *ptr = (char *)str;
  while (*ptr) {
    ptr++;
  }
  return (ptr - str);
}

void uart_dma_print(const char *str) {
  while (!dma_transfer_done)
    ;

  uart_dma_start((uint8_t *)str, _strlen(str));
}