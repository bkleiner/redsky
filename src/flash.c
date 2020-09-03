#include "flash.h"

#include "debug.h"

#define FLASH_MAGIC 0xdead
#define FLASH_MAGIC_ADDR (FLASH_PAGE_SIZE - sizeof(uint16_t))

#define FCTL_BUSY (1 << 7)
#define FCTL_SWBUSY (1 << 6)
#define FCTL_WRITE (1 << 1)
#define FCTL_ERASE (1 << 0)

__code __at(FLASH_PAGE_OFFSET) uint8_t flash_storage_page[FLASH_PAGE_SIZE] = {0x0};

void flash_init() {
  debug_print("flash_init\r\n");

  uint16_t magic = 0xFFFF;
  flash_read(FLASH_MAGIC_ADDR, (uint8_t *)&magic, sizeof(uint16_t));

  if (magic != FLASH_MAGIC) {
    magic = FLASH_MAGIC;
    flash_write(FLASH_MAGIC_ADDR, (uint8_t *)&magic, sizeof(uint16_t));
  }
}

void flash_read(uint16_t addr, uint8_t *buf, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    buf[i] = flash_storage_page[addr + i];
  }
}

void flash_set_erase() {
  __asm__(".even\n ORL _FCTL,#0x01\nNOP");
}

void flash_set_write() {
  __asm__(".even\n ORL _FCTL,#0x02\nNOP");
}

void flash_write(uint16_t addr, uint8_t *buf, uint16_t len) {
  EA = 0;

  while (FCTL & FCTL_BUSY)
    ;

  FWT = 0x2a;
  SET_WORD(FADDRH, FADDRL, addr >> 1);
  flash_set_erase();

  while (FCTL & FCTL_BUSY)
    ;

  flash_set_write();

  while (FCTL & (FCTL_SWBUSY | FCTL_BUSY))
    ;

  for (uint8_t i = 0; i < len; i += 2) {
    SET_WORD(FADDRH, FADDRL, (addr + i) >> 1);

    FWDATA = buf[i];
    FWDATA = buf[i + 1];

    while (FCTL & FCTL_SWBUSY)
      ;
  }

  EA = 1;
}
