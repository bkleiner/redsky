#include "protocol/protocol.h"

#include "delay.h"
#include "driver.h"
#include "radio.h"

#include "frsky.h"
#include "redpine.h"

EXT_MEMORY uint8_t current_channel_index = 0;

EXT_MEMORY uint8_t fscal1_table[HOPTABLE_SIZE];
EXT_MEMORY uint8_t fscal2;
EXT_MEMORY uint8_t fscal3;

bind_data EXT_MEMORY bind = {
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
  radio_write_reg(PKTCTRL1, 0x0D);

  delay_ms(1);
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