#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#define WRITE_FLAG 0x00
#define READ_FLAG 0x80
#define BURST_FLAG 0x40

#define PARTNUM (0x30 | BURST_FLAG)
#define VERSION (0x31 | BURST_FLAG)
#define FREQEST (0x32 | BURST_FLAG)
#define LQI (0x33 | BURST_FLAG)
#define RSSI (0x34 | BURST_FLAG)
#define MARCSTATE (0x35 | BURST_FLAG)
#define WORTIME1 (0x36 | BURST_FLAG)
#define WORTIME0 (0x37 | BURST_FLAG)
#define PKTSTATUS (0x38 | BURST_FLAG)
#define VCO_VC_DAC (0x39 | BURST_FLAG)
#define TXBYTES (0x3A | BURST_FLAG)
#define RXBYTES (0x3B | BURST_FLAG)
#define RCCTRL1_STATUS (0x3C | BURST_FLAG)
#define RCCTRL0_STATUS (0x3D | BURST_FLAG)

#define RFST_SRES 0x30
#define RFST_SFSTXON 0x31
#define RFST_SXOFF 0x32
#define RFST_SCAL 0x33
#define RFST_SRX 0x34
#define RFST_STX 0x35
#define RFST_SIDLE 0x36
#define RFST_SWOR 0x38
#define RFST_SPWD 0x39
#define RFST_SFRX 0x3A
#define RFST_SFTX 0x3B
#define RFST_SWORRST 0x3C
#define RFST_SNOP 0x3D

#define IOCFG2 0x00
#define IOCFG1 0x01
#define IOCFG0 0x02
#define FIFOTHR 0x03
#define SYNC1 0x04
#define SYNC0 0x05
#define PKTLEN 0x06
#define PKTCTRL1 0x07
#define PKTCTRL0 0x08
#define ADDR 0x09
#define CHANNR 0x0A
#define FSCTRL1 0x0B
#define FSCTRL0 0x0C
#define FREQ2 0x0D
#define FREQ1 0x0E
#define FREQ0 0x0F
#define MDMCFG4 0x10
#define MDMCFG3 0x11
#define MDMCFG2 0x12
#define MDMCFG1 0x13
#define MDMCFG0 0x14
#define DEVIATN 0x15
#define MCSM2 0x16
#define MCSM1 0x17
#define MCSM0 0x18
#define FOCCFG 0x19
#define BSCFG 0x1A
#define AGCCTRL2 0x1B
#define AGCCTRL1 0x1C
#define AGCCTRL0 0x1D
#define WOREVT1 0x1E
#define WOREVT0 0x1F
#define WORCTRL 0x20
#define FREND1 0x21
#define FREND0 0x22
#define FSCAL3 0x23
#define FSCAL2 0x24
#define FSCAL1 0x25
#define FSCAL0 0x26
#define RCCTRL1 0x27
#define RCCTRL0 0x28
#define FSTEST 0x29
#define PTEST 0x2A
#define AGCTEST 0x2B
#define TEST2 0x2C
#define TEST1 0x2D
#define TEST0 0x2E
#define PA_TABLE0 0x3E

uint8_t radio_read_reg(uint8_t reg);
uint8_t radio_write_reg(uint8_t reg, uint8_t val);
void radio_strobe(uint8_t val);

void radio_init();
void radio_io_config();
void radio_enable_rx();

void radio_handle_overflows();

uint8_t radio_received_packet();
void radio_reset_packet();

void radio_switch_antenna();

#endif