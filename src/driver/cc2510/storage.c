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
  if ((len & 0x1) == 0x1) {
    len++;
  }

  flash_erase(FLASH_PAGE_OFFSET / FLASH_PAGE_SIZE);
  flash_write(FLASH_PAGE_OFFSET, buf, len);

  dma_init();
  radio_init();
}