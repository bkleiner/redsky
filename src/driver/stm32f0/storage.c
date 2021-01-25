#include "storage.h"

#include "driver.h"

#include <string.h>

#define STORAGE_FLASH_ADDR 0x08007000

static void storage_wait_for_ready() {
  while ((FLASH->SR & FLASH_SR_BSY) != 0)
    ;
}

static void storage_unlock() {
  if ((FLASH->CR & FLASH_CR_LOCK) != 0) {
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  }
}

static void storage_lock() {
  FLASH->CR |= FLASH_CR_LOCK;
}

static void storage_erase_page() {
  storage_wait_for_ready();

  FLASH->CR |= FLASH_CR_PER;
  FLASH->AR = STORAGE_FLASH_ADDR;
  FLASH->CR |= FLASH_CR_STRT;

  storage_wait_for_ready();

  FLASH->CR &= ~FLASH_CR_PER;
}

static void storage_program(uint16_t offset, uint16_t data) {
  FLASH->CR |= FLASH_CR_PG;

  *(volatile uint16_t *)(STORAGE_FLASH_ADDR + offset) = data;

  storage_wait_for_ready();
  if ((FLASH->SR & FLASH_SR_EOP) != 0) {
    FLASH->SR = FLASH_SR_EOP;
  }

  FLASH->CR &= ~FLASH_CR_PG;
}

void storage_init() {
}

void storage_read(uint8_t *buf, uint16_t len) {
  uint16_t full_len = len;
  if (len & 0x1) {
    full_len += 1;
  }
  uint16_t half_len = full_len / 2;

  storage_wait_for_ready();

  uint16_t tmp_buf[half_len];
  memset(tmp_buf, 0, half_len);

  for (uint16_t i = 0; i < (half_len); i++) {
    tmp_buf[i] = *(volatile uint16_t *)(STORAGE_FLASH_ADDR + i * 2);
  }

  memcpy(buf, tmp_buf, len);
}

void storage_write(uint8_t *buf, uint16_t len) {
  storage_wait_for_ready();
  storage_unlock();

  storage_erase_page();

  uint16_t full_len = len;
  if (len & 0x1) {
    full_len += 1;
  }
  uint16_t half_len = full_len / 2;

  uint16_t tmp_buf[half_len];
  memcpy(tmp_buf, buf, len);

  for (uint16_t i = 0; i < (half_len); i++) {
    storage_program(i * 2, tmp_buf[i]);
  }

  storage_lock();
}