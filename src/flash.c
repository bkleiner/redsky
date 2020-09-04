#include "flash.h"

#include "debug.h"
#include "delay.h"
#include "dma.h"

#define FLASH_MAGIC 0xdead
#define FLASH_MAGIC_ADDR 0

#define FCTL_BUSY (1 << 7)
#define FCTL_SWBUSY (1 << 6)
#define FCTL_WRITE (1 << 1)
#define FCTL_ERASE (1 << 0)

__code __at(FLASH_PAGE_OFFSET) uint8_t flash_storage_page[FLASH_PAGE_SIZE] = {0x0};

void flash_init() {
  debug_print("flash_init\r\n");
}

void flash_read(uint16_t addr, uint8_t *buf, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    buf[i] = flash_storage_page[addr + i];
  }
}

void flash_start_write() {
  __asm__(".even\nMOV _FCTL,#0x02\nNOP");
}

void flash_start_erase() {
  __asm__(".even\nMOV _FCTL,#0x01\nNOP");
}

void flash_write(uint16_t offset, uint8_t *buf, uint16_t len) __critical {
  EA = 0;

  SET_WORD(dma_desc[2].SRCADDRH, dma_desc[2].SRCADDRL, buf);
  SET_WORD(dma_desc[2].LENH, dma_desc[2].LENL, len);
  SET_WORD(dma_desc[2].DESTADDRH, dma_desc[2].DESTADDRL, 0xDFAF);

  dma_desc[2].VLEN = 0x0;
  dma_desc[2].WORDSIZE = 0x0;
  dma_desc[2].TMODE = 0x0;

  dma_desc[2].TRIG = 18;
  dma_desc[2].SRCINC = 0x1;
  dma_desc[2].DESTINC = 0x0;

  dma_desc[2].IRQMASK = 0x0;
  dma_desc[2].M8 = 0x0;
  dma_desc[2].PRIORITY = 0x2;

  SET_WORD(DMA1CFGH, DMA1CFGL, &dma_desc[1]);

  while (FCTL & FCTL_BUSY)
    ;

  FWT = 0x2a;
  SET_WORD(FADDRH, FADDRL, ((uint16_t)flash_storage_page) >> 1);
  flash_start_erase();

  while (FCTL & FCTL_BUSY)
    ;

  DMAIRQ = 0;
  DMAARM = 0x80 | 0x1F;
  delay_45_nop();

  DMAARM = DMA_CH2;
  delay_45_nop();

  flash_start_write();
  DMAREQ |= DMA_CH2;

  while ((DMAIRQ & DMA_CH2) == 0)
    ;

  while (FCTL & (FCTL_SWBUSY | FCTL_BUSY))
    ;

  DMAIRQ &= ~DMA_CH2;
  EA = 1;
}
