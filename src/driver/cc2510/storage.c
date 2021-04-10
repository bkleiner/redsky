#include "storage.h"

#include "dma.h"
#include "flash.h"
#include "radio.h"

#include <string.h>

__code __at(FLASH_PAGE_OFFSET) uint8_t flash_storage_page[FLASH_PAGE_SIZE] = {0x0};

void storage_init() {
  flash_init();
}

void storage_read(uint8_t *buf, uint16_t len) {
  memcpy(buf, flash_storage_page, len);
}

void storage_write(uint8_t *buf, uint16_t len) {
  __xdata uint8_t tmp_buf[128];
  if (len > 128) {
    return;
  }

  for (uint16_t i = 0; i < 128; i++) {
    tmp_buf[i] = 0;
  }
  for (uint16_t i = 0; i < len; i++) {
    tmp_buf[i] = buf[i];
  }

  flash_erase(FLASH_PAGE_OFFSET / FLASH_PAGE_SIZE);
  flash_write(FLASH_PAGE_OFFSET, tmp_buf, 128);

  dma_init();
  radio_init();
}