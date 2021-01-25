#include "radio.h"

#include <string.h>

#include "delay.h"
#include "driver.h"
#include "gpio.h"
#include "redpine.h"

#define FIFO 0x3F

#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE

volatile uint8_t packet[RADIO_RX_BUF_SIZE];
static volatile uint8_t received_packet = 0;
static volatile uint8_t dma_transfer_done = 1;
static uint8_t current_antenna = 0;

static void set_antenna(uint8_t index) {
  if (index == 0) {
    gpio_set(RF_SWANT_PIN);
  } else {
    gpio_reset(RF_SWANT_PIN);
  }
  current_antenna = index;
}

void radio_init() {
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);

  gpio_config(CC2500_CSN_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_PULL_UP);

  gpio_config(CC2500_SCK_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_AF | GPIO_PULL_DOWN);
  gpio_config_af(CC2500_SCK_PIN, 0);
  gpio_config(CC2500_MISO_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_AF | GPIO_PULL_DOWN);
  gpio_config_af(CC2500_MISO_PIN, 0);
  gpio_config(CC2500_MOSI_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_AF | GPIO_PULL_DOWN);
  gpio_config_af(CC2500_MOSI_PIN, 0);

  gpio_config(RF_LNA_PIN, GPIO_OUTPUT | GPIO_PP);
  gpio_set(RF_LNA_PIN);

  gpio_config(RF_PA_PIN, GPIO_OUTPUT | GPIO_PP);
  gpio_reset(RF_PA_PIN);

  gpio_config(RF_BYPASS_PIN, GPIO_OUTPUT | GPIO_PP);
  gpio_reset(RF_BYPASS_PIN);

  gpio_config(RF_SWANT_PIN, GPIO_OUTPUT | GPIO_PP);
  set_antenna(0);

  gpio_config(CC2500_GDO0_PIN, GPIO_INPUT | GPIO_OD);
  MODIFY_REG(SYSCFG->EXTICR[0], SYSCFG_EXTICR1_EXTI1, SYSCFG_EXTICR1_EXTI1_PB);
  SET_BIT(EXTI->IMR, EXTI_IMR_MR1);
  SET_BIT(EXTI->RTSR, EXTI_RTSR_TR1);
  CLEAR_BIT(EXTI->FTSR, EXTI_FTSR_TR1);

  SPI1->CR1 &= ~SPI_CR1_SPE;
  SPI1->CR1 = SPI_CR1_BR_1 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
  SPI1->CR2 = SPI_CR2_RXNEIE | SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
  SPI1->CR1 |= SPI_CR1_SPE;

  radio_strobe(RFST_SRES);
  delay_ms(100);
  radio_strobe(RFST_SIDLE);
  delay_ms(100);

  NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  DMA1->IFCR |= DMA_ISR_TCIF2;
}

void radio_io_config() {
  radio_write_reg(IOCFG0, 0x01);
  radio_write_reg(IOCFG2, 0x01);

  radio_write_reg(FIFOTHR, 0x07);

  NVIC_EnableIRQ(EXTI0_1_IRQn);
  SET_BIT(EXTI->PR, EXTI_PR_PR1);
}

static void radio_csn_enable() {
  gpio_reset(CC2500_CSN_PIN);
}

static void radio_csn_disable() {
  gpio_set(CC2500_CSN_PIN);
}

static uint8_t radio_transfer(uint8_t val) {
  for (uint16_t timeout = 0x2000; (SPI1->SR & SPI_SR_TXE) == 0; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  *((__IO uint8_t *)&(SPI1->DR)) = val;

  for (uint16_t timeout = 0x2000; (SPI1->SR & SPI_SR_RXNE) == 0; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  for (uint16_t timeout = 0x2000; (SPI1->SR & SPI_SR_BSY) != 0; timeout--) {
    if (timeout == 0) {
      return 0;
    }
  }

  return *((__IO uint8_t *)&(SPI1->DR));
}

uint8_t radio_write_reg(uint8_t reg, uint8_t val) {
  while (!dma_transfer_done)
    ;

  radio_csn_enable();
  radio_transfer(reg | WRITE_FLAG);
  uint8_t res = radio_transfer(val);
  radio_csn_disable();
  return res;
}

uint8_t radio_read_reg(uint8_t reg) {
  return radio_write_reg(reg | READ_FLAG, 0xFF);
}

void radio_strobe(uint8_t val) {
  while (!dma_transfer_done)
    ;

  radio_csn_enable();
  radio_transfer(val);
  radio_csn_disable();
}

static inline uint8_t radio_read_multi(uint8_t reg, uint8_t *result, uint8_t len) {
  while (!dma_transfer_done)
    ;

  radio_csn_enable();

  const uint8_t ret = radio_transfer(reg);
  for (uint8_t i = 0; i < len; i++) {
    result[i] = radio_transfer(0xFF);
  }

  radio_csn_disable();
  return ret;
}

static volatile uint8_t dma_buffer[128];

static uint8_t packet_size() {
  if (gpio_read(CC2500_GDO0_PIN) == 0) {
    return 0;
  }

  // there is a bug in the cc2500
  // see p3 http:// www.ti.com/lit/er/swrz002e/swrz002e.pdf
  // workaround: read len register very quickly twice:

  // try this 10 times befor giving up:
  for (uint8_t i = 0; i < 10; i++) {
    uint8_t len1 = radio_read_reg(RXBYTES) & 0x7F;
    uint8_t len2 = radio_read_reg(RXBYTES) & 0x7F;

    // valid len found?
    if (len1 == len2) {
      return len1;
    }
  }

  return 0;
}

static void radio_start_read_fifo() {
  if (!dma_transfer_done) {
    return;
  }

  const uint8_t len = packet_size();
  if (len == 0 || len >= 128) {
    return;
  }

  dma_transfer_done = 0;
  dma_buffer[0] = FIFO | READ_FLAG | BURST_FLAG;
  memset(dma_buffer + 1, 0xFF, len);

  SPI1->CR1 &= ~SPI_CR1_SPE;

  const uint8_t full_len = len + 1;

  // RX Channel
  DMA1_Channel2->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_TCIE;
  DMA1_Channel2->CPAR = (uint32_t)(&SPI1->DR);
  DMA1_Channel2->CMAR = (uint32_t)(&dma_buffer);
  DMA1_Channel2->CNDTR = full_len;

  // TX Channel
  DMA1_Channel3->CCR = DMA_CCR_PL_1 | DMA_CCR_MINC | DMA_CCR_DIR;
  DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR);
  DMA1_Channel3->CMAR = (uint32_t)(&dma_buffer);
  DMA1_Channel3->CNDTR = full_len;

  // FIRE!
  radio_csn_enable();
  DMA1_Channel2->CCR |= DMA_CCR_EN;
  DMA1_Channel3->CCR |= DMA_CCR_EN;
  SPI1->CR2 &= ~SPI_CR2_FRXTH;
  SPI1->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;
  SPI1->CR1 |= SPI_CR1_SPE;
}

void DMA1_Channel2_3_IRQHandler() {
  if (DMA1->ISR & DMA_ISR_TCIF2) {
    DMA1->IFCR |= DMA_ISR_TCIF2;

    SPI1->CR1 &= ~SPI_CR1_SPE;
    SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    DMA1_Channel3->CCR &= ~DMA_CCR_EN;
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;
    SPI1->CR2 |= SPI_CR2_FRXTH;
    SPI1->CR1 |= SPI_CR1_SPE;

    radio_csn_disable();

    memcpy(packet, dma_buffer + 1, RADIO_RX_BUF_SIZE);

    dma_transfer_done = 1;
    received_packet = 1;
  }
}

void EXTI0_1_IRQHandler() {
  if (EXTI->PR & EXTI_PR_PR1) {
    SET_BIT(EXTI->PR, EXTI_PR_PR1);
    radio_start_read_fifo();
  }
}

uint8_t radio_received_packet() {
  return received_packet;
}

void radio_reset_packet() {
  received_packet = 0;
}

void radio_switch_antenna() {
#ifdef USE_FIXED_ANTENNA
  set_antenna(USE_FIXED_ANTENNA);
#else
  set_antenna(current_antenna == 0 ? 1 : 0);
#endif
}

void radio_enable_rx() {
}

void radio_handle_overflows() {
  volatile uint8_t marc_state = radio_read_reg(MARCSTATE) & 0x1F;
  if (marc_state == 0x11) {
    radio_strobe(RFST_SFRX);
  }
}