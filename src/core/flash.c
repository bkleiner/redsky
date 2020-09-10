#include "flash.h"

#include "delay.h"

#include <string.h>

#define FLASH_MAGIC 0xdead
#define FLASH_MAGIC_ADDR 0

static __xdata dma_desc_t flash_dma_desc;

void flash_init() {
}

void flash_read(uint16_t addr, uint8_t *buf, uint16_t len) {
  __code uint8_t *ptr = (__code uint8_t *)addr;

  memcpy(buf, ptr, len);
}

void flash_start_write() {
  __asm__(".even\nMOV _FCTL,#0x02\nNOP");
}

void flash_start_erase() {
  __asm__(".even\nMOV _FCTL,#0x01\nNOP");
}

void flash_erase(uint16_t page) {
  while (FCTL & FCTL_BUSY)
    ;

  FWT = 0x2a;
  SET_WORD(FADDRH, FADDRL, page >> 1);
  flash_start_erase();

  while (FCTL & FCTL_BUSY)
    ;
}

void flash_write(uint16_t offset, uint8_t *buf, uint16_t len) __critical {
  if ((len & 0x1) == 1) {
    len++;
  }

  SET_WORD(flash_dma_desc.SRCADDRH, flash_dma_desc.SRCADDRL, buf);
  SET_WORD(flash_dma_desc.LENH, flash_dma_desc.LENL, len);
  SET_WORD(flash_dma_desc.DESTADDRH, flash_dma_desc.DESTADDRL, 0xDFAF);

  flash_dma_desc.VLEN = 0x0;
  flash_dma_desc.WORDSIZE = 0x0;
  flash_dma_desc.TMODE = 0x0;

  flash_dma_desc.TRIG = 18;
  flash_dma_desc.SRCINC = 0x1;
  flash_dma_desc.DESTINC = 0x0;

  flash_dma_desc.IRQMASK = 0x0;
  flash_dma_desc.M8 = 0x0;
  flash_dma_desc.PRIORITY = 0x2;

  SET_WORD(DMA0CFGH, DMA0CFGL, &flash_dma_desc);

  while (FCTL & FCTL_BUSY)
    ;

  FWT = 0x2a;
  SET_WORD(FADDRH, FADDRL, offset >> 1);

  DMAIRQ = 0;
  DMAARM = 0x80 | 0x1F;
  delay_45_nop();

  DMAARM = DMA_CH0;
  delay_45_nop();

  flash_start_write();
  DMAREQ |= DMA_CH0;

  while ((DMAIRQ & DMA_CH0) == 0)
    ;

  while (FCTL & (FCTL_SWBUSY | FCTL_BUSY))
    ;

  DMAIRQ &= ~DMA_CH0;
}
