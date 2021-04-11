#include "protocol/protocol.h"

#include <string.h>

#include "delay.h"
#include "driver.h"
#include "radio.h"
#include "uart_dma.h"

EXT_MEMORY uint8_t current_channel_index = 0;

EXT_MEMORY uint8_t fscal1_table[HOPTABLE_SIZE];
EXT_MEMORY uint8_t fscal2;
EXT_MEMORY uint8_t fscal3;

EXT_MEMORY bind_data_t bind = {
    .txid = {0xFF, 0xFF},
};

void protocol_tune_freq(int8_t freq) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(FSCTRL0, freq);
  delay_ms(1);
  radio_strobe(RFST_SRX);
}

void protocol_tune_channel(uint8_t ch) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(CHANNR, ch);
  radio_strobe(RFST_SCAL);

  while (radio_read_reg(MARCSTATE) != 0x01)
    ;
}

void protocol_enter_rxmode(uint8_t channel) {
  radio_strobe(RFST_SIDLE);

  protocol_tune_channel(channel);
  radio_enable_rx();

  radio_strobe(RFST_SRX);
}

void protocol_set_channel(uint8_t hop_index) {
  uint8_t ch = bind.hop_table[hop_index];

  radio_strobe(RFST_SIDLE);

  radio_write_reg(FSCAL3, fscal3);
  radio_write_reg(FSCAL2, fscal2);
  radio_write_reg(FSCAL1, fscal1_table[hop_index]);

  radio_write_reg(CHANNR, ch);
}

void protocol_increment_channel(int8_t cnt) {
  int8_t next = current_channel_index + cnt;

  // convert to a safe unsigned number:
  if (next < 0) {
    next += HOPTABLE_SIZE;
  }
  if (next >= HOPTABLE_SIZE) {
    next -= HOPTABLE_SIZE;
  }

  current_channel_index = next;
  protocol_set_channel(current_channel_index);
}

void protocol_calibrate() {
  protocol_tune_freq(bind.freq_offset);

  for (int i = 0; i < HOPTABLE_SIZE; i++) {
    uint8_t ch = bind.hop_table[i];
    protocol_tune_channel(ch);
    fscal1_table[i] = radio_read_reg(FSCAL1);
  }

  fscal3 = radio_read_reg(FSCAL3);
  fscal2 = radio_read_reg(FSCAL2);

  radio_strobe(RFST_SIDLE);
}

void protocol_set_address() {
  radio_strobe(RFST_SIDLE);

  // freq offset
  radio_write_reg(FSCTRL0, (uint8_t)bind.freq_offset);

  // never automatically calibrate, po_timeout count = 64
  // no autotune as (we use our pll map)
  radio_write_reg(MCSM0, 0x8);
  radio_write_reg(ADDR, bind.txid[0]);

  // ADR_CHK, APPEND_STATUS, CRC_AUTOFLUSH
  radio_write_reg(PKTCTRL1, 0x05);

  delay_ms(1);
}

void protocol_send_sbus(uint16_t *channel, uint8_t packet_lost, uint8_t failsafe) {
  static EXT_MEMORY uint8_t sbus_data[SBUS_SIZE];
  memset(sbus_data, 0, SBUS_SIZE);

  sbus_data[0] = SBUS_SYNC;

  sbus_data[1] = channel[0];                                // 0000 0000
  sbus_data[2] = (channel[1] << 3) | channel[0] >> 8;       // 1111 1000
  sbus_data[3] = (channel[1] >> 5) | (channel[2] << 6);     // 2211 1111
  sbus_data[4] = (channel[2] >> 2) & 0xFF;                  // 2222 2222
  sbus_data[5] = (channel[2] >> 10) | (channel[3] << 1);    // 3333 3332
  sbus_data[6] = (channel[3] >> 7) | (channel[4] << 4);     // 4444 3333
  sbus_data[7] = (channel[4] >> 4) | (channel[5] << 7);     // 5444 4444
  sbus_data[8] = (channel[5] >> 1) & 0xFF;                  // 5555 5555
  sbus_data[9] = (channel[5] >> 9) | (channel[6] << 2);     // 6666 6655
  sbus_data[10] = (channel[6] >> 6) | (channel[7] << 5);    // 7776 6666
  sbus_data[11] = (channel[7] >> 3) & 0xFF;                 // 7777 7777
  sbus_data[12] = channel[8] & 0xFF;                        // 8888 8888
  sbus_data[13] = (channel[9] << 3) | channel[8];           // 9999 9888
  sbus_data[14] = (channel[9] >> 5) | channel[10];          // 1199 9999
  sbus_data[15] = (channel[10] >> 2) & 0xFF;                // 1111 1111
  sbus_data[16] = (channel[10] >> 10) | (channel[11] << 1); // 2222 2221
  sbus_data[17] = (channel[11] >> 7) | (channel[12] << 4);  // 3333 2222
  sbus_data[18] = (channel[12] >> 4) | (channel[13] << 7);  // 4333 3333
  sbus_data[19] = (channel[13] >> 1) & 0xFF;                // 4444 4444
  sbus_data[20] = (channel[13] >> 9) | (channel[14] << 2);  // 5555 5544
  sbus_data[21] = (channel[14] >> 6) | (channel[15] << 5);  // 6665 5555
  sbus_data[22] = (channel[15] >> 3) & 0xFF;                // 6666 6666

  sbus_data[23] = 0;
  if (failsafe) {
    sbus_data[23] |= SBUS_FLAG_FAILSAFE_ACTIVE;
  } else if (packet_lost) {
    sbus_data[23] |= SBUS_FLAG_FRAME_LOST;
  }

  sbus_data[24] = 0;

  uart_dma_start(sbus_data, SBUS_SIZE);
}

void protocol_init() {
#ifdef PROTOCOL_REDPINE
  redpine_init();
#endif

#ifdef PROTOCOL_FRSKY
  frsky_init();
#endif
}

void protocol_main() {
#ifdef PROTOCOL_REDPINE
  redpine_main();
#endif

#ifdef PROTOCOL_FRSKY
  frsky_main();
#endif
}