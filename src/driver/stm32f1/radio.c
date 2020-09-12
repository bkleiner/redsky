#include "radio.h"

#include "delay.h"
#include "driver.h"
#include "gpio.h"
#include "redpine.h"

#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE

volatile uint8_t packet[RADIO_RX_BUF_SIZE];
static volatile uint8_t received_packet = 0;

void radio_init() {
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);

  gpio_config(CC2500_GPIO_PORT, CC2500_CSN_PIN, GPIO_OUTPUT | GPIO_PP);
  gpio_config(CC2500_GPIO_PORT, CC2500_SCK_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_AF);
  gpio_config(CC2500_GPIO_PORT, CC2500_MISO_PIN, GPIO_INPUT | GPIO_OD | GPIO_AF);
  gpio_config(CC2500_GPIO_PORT, CC2500_MOSI_PIN, GPIO_OUTPUT | GPIO_PP | GPIO_AF);

  SPI1->CR1 = SPI_CR1_BR_1 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
  SPI1->I2SCFGR &= ~SPI_I2SCFGR_I2SMOD;
  SPI1->CRCPR = 7;
  SPI1->CR1 |= SPI_CR1_SPE;

  while ((SPI1->SR & SPI_SR_TXE) == 0)
    ;

  uint8_t dummy = SPI1->DR;
  dummy = dummy + 1;
}

static void radio_csn_enable() {
  gpio_reset(CC2500_GPIO_PORT, CC2500_CSN_PIN);
  delay_us(1);
}

static void radio_csn_disable() {
  delay_us(1);
  gpio_set(CC2500_GPIO_PORT, CC2500_CSN_PIN);
}

static uint8_t radio_transfer(uint8_t val) {
  while ((SPI1->SR & SPI_SR_TXE) == 0)
    ;

  SPI1->DR = val;

  while ((SPI1->SR & SPI_SR_RXNE) == 0)
    ;

  return SPI1->DR;
}

uint8_t radio_write_reg(uint8_t reg, uint8_t val) {
  radio_csn_enable();
  radio_transfer(reg | WRITE_FLAG);
  uint8_t res = radio_transfer(val);
  radio_csn_disable();
  return res;
}

uint8_t radio_read_reg(uint8_t reg) {
  return radio_write_reg(reg, 0xFF);
}

void radio_strobe(uint8_t val) {
  radio_csn_enable();
  radio_transfer(val);
  radio_csn_disable();
}

uint8_t radio_received_packet() {
  return received_packet;
}

void radio_reset_packet() {
  received_packet = 0;
}

void radio_switch_antenna() {
}

void radio_enable_rx() {
}