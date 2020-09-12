#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#include "driver.h"

#define radio_read_reg(reg) reg
#define radio_write_reg(reg, val) reg = val
#define radio_strobe(val) RFST = val

void radio_init();
void radio_enable_rx();

uint8_t radio_received_packet();
void radio_reset_packet();

void radio_switch_antenna();

void radio_isr(void) __interrupt(RF_VECTOR);

#endif