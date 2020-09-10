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
  flash_erase((uint16_t)flash_storage_page);
  flash_write((uint16_t)flash_storage_page, buf, len);

  dma_init();
  radio_init();
}