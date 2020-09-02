#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#include "cc25xx.h"

#define RFST_SNOP 0x05
#define RFST_SIDLE 0x04
#define RFST_STX 0x03
#define RFST_SRX 0x02
#define RFST_SCAL 0x01
#define RFST_SFSTXON 0x00

SFRX(TEST0, 0xDF25);
SFRX(TEST1, 0xDF24);
SFRX(TEST2, 0xDF23);

#define radio_read_reg(reg) reg
#define radio_write_reg(reg, val) reg = val
#define radio_strobe(val) RFST = val

void radio_init();
void radio_enable_rx();

uint8_t radio_received_packet();
void radio_reset_packet();

void radio_dma_isr();
void radio_isr(void) __interrupt(RF_VECTOR);

#endif