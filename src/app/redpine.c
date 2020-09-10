#include "redpine.h"

#include "debug.h"
#include "delay.h"
#include "led.h"
#include "radio.h"
#include "storage.h"
#include "timer.h"

#define HOPTABLE_SIZE 49
#define MAX_HOPTABLE_SIZE 50
#define MAX_BIND_PACKET_COUNT 10

#define DEFAULT_PACKET_TIME_US 50000

#define CHANNEL_START 3

#define HOPDATA_RECEIVE_DONE ((1 << (MAX_BIND_PACKET_COUNT)) - 1)

#define REDPINE_VALID_PACKET_CRC(_b) (_b[REDPINE_PACKET_SIZE_W_ADDONS - 1] & (1 << 7))
#define REDPINE_VALID_PACKET_BIND(_b) ((_b[2] == 0x01) && REDPINE_VALID_PACKET_CRC(_b))

#define REDPINE_VALID_TXID(_b) ((_b[1] == bind.txid[0]) && (_b[2] == bind.txid[1]))
#define REDPINE_VALID_PACKET(_b) ((_b[0] == 10) && REDPINE_VALID_TXID(_b) && REDPINE_VALID_PACKET_CRC(_b))

typedef struct {
  uint8_t txid[2];
  uint8_t hop_table[MAX_HOPTABLE_SIZE];
  int8_t freq_offset;
} bind_data;

typedef enum {
  TUNE_INIT,
  TUNE_FAST_SEARCH,
  TUNE_SLOW_SEARCH,
  TUNE_DONE
} tune_state;

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

__xdata bind_data bind = {
    .txid = {0xFF, 0xFF},
};

extern volatile __xdata uint8_t packet[REDPINE_PACKET_BUFFER_SIZE];

__xdata uint8_t current_channel_index = 0;

__xdata uint8_t fscal1_table[HOPTABLE_SIZE];
__xdata uint8_t fscal2;
__xdata uint8_t fscal3;

void redpine_configure() {
  debug_print("redpine_configure\r\n");

  //radio_write_reg(FIFOTHR, 0x07);

  radio_write_reg(PKTLEN, REDPINE_PACKET_SIZE);
  radio_write_reg(PKTCTRL1, 0x0C);
  radio_write_reg(PKTCTRL0, 0x05);
  radio_write_reg(PA_TABLE0, 0xFF);

  radio_write_reg(FSCTRL1, 0x0A);
  radio_write_reg(FSCTRL0, 0x00);

  radio_write_reg(FREQ2, 0x5D);
  radio_write_reg(FREQ1, 0x93);
  radio_write_reg(FREQ0, 0xB1);

  radio_write_reg(MDMCFG4, 0x2D);
  radio_write_reg(MDMCFG3, 0x3B);
  radio_write_reg(MDMCFG2, 0x73);
  radio_write_reg(MDMCFG1, 0x23);
  radio_write_reg(MDMCFG0, 0x56);

  radio_write_reg(DEVIATN, 0x00);

  radio_write_reg(MCSM1, 0x0C);
  radio_write_reg(MCSM0, 0x18);

  radio_write_reg(FOCCFG, 0x1D);
  radio_write_reg(BSCFG, 0x1C);

  radio_write_reg(AGCCTRL2, 0xC7);
  radio_write_reg(AGCCTRL1, 0x00);
  radio_write_reg(AGCCTRL0, 0xB0);

  radio_write_reg(FREND1, 0xB6);
  radio_write_reg(FREND0, 0x10);

  radio_write_reg(FSCAL3, 0xEA);
  radio_write_reg(FSCAL2, 0x0A);
  radio_write_reg(FSCAL1, 0x00);
  radio_write_reg(FSCAL0, 0x11);

  //radio_write_reg(FSTEST, 0x59);

  radio_write_reg(TEST2, 0x88);
  radio_write_reg(TEST1, 0x31);
  radio_write_reg(TEST0, 0x0B);

  radio_write_reg(ADDR, 0x00);
}

void redpine_tune_freq(int8_t freq) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(FSCTRL0, freq);
  delay_ms(1);
  radio_strobe(RFST_SRX);
}

void redpine_tune_channel(uint8_t ch) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(CHANNR, ch);
  radio_strobe(RFST_SCAL);

  while (radio_read_reg(MARCSTATE) != 0x01)
    ;
}

void redpine_enter_rxmode(uint8_t channel) {
  radio_strobe(RFST_SIDLE);

  redpine_tune_channel(channel);
  radio_enable_rx();

  radio_strobe(RFST_SRX);
}

void redpine_set_channel(uint8_t hop_index) {
  uint8_t ch = bind.hop_table[hop_index];

  radio_strobe(RFST_SIDLE);

  radio_write_reg(FSCAL3, fscal3);
  radio_write_reg(FSCAL2, fscal2);
  radio_write_reg(FSCAL1, fscal1_table[hop_index]);

  radio_write_reg(CHANNR, ch);
}

void redpine_increment_channel(int8_t cnt) {
  int8_t next = current_channel_index + cnt;

  // convert to a safe unsigned number:
  if (next < 0) {
    next += HOPTABLE_SIZE;
  }
  if (next >= HOPTABLE_SIZE) {
    next -= HOPTABLE_SIZE;
  }

  current_channel_index = next;
  redpine_set_channel(current_channel_index);
}

inline uint8_t redpine_handle_overflows() {
  uint8_t marc_state = radio_read_reg(MARCSTATE) & 0x1F;
  if (marc_state == 0x11) {
    // debug_print("redpine_rx_overflow\r\n");

    radio_strobe(RFST_SIDLE);
    while (radio_read_reg(MARCSTATE) != 0x01)
      ;

    radio_reset_packet();
    radio_enable_rx();

    radio_strobe(RFST_SRX);
    return 1;
  }

  return 0;
}

void redpine_tune() {
  debug_print("redpine_tune\r\n");

  bind.freq_offset = 0;

  radio_write_reg(FOCCFG, 0x14);

  radio_write_reg(FSCTRL0, (uint8_t)bind.freq_offset);
  radio_write_reg(PKTCTRL1, 0x0C);
  radio_write_reg(MCSM0, 0x8);

  redpine_enter_rxmode(0);

  int8_t fscal0_min = 127;
  int8_t fscal0_max = -127;

  uint8_t done = 0;
  uint8_t received_bind_packet = 0;

  tune_state state = TUNE_INIT;

  while (state != TUNE_DONE) {
    switch (state) {
    case TUNE_INIT:
      bind.freq_offset = -127;
      state = TUNE_FAST_SEARCH;
      break;

    case TUNE_FAST_SEARCH:
      if (bind.freq_offset < 127 - 10) {
        bind.freq_offset += 9;
      } else {
        if (received_bind_packet) {
          bind.freq_offset = fscal0_min - 9;
          state = TUNE_SLOW_SEARCH;
        } else {
          state = TUNE_INIT;
        }
      }
      break;

    case TUNE_SLOW_SEARCH:
      if (bind.freq_offset < fscal0_max + 9) {
        bind.freq_offset++;
      } else {
        state = TUNE_DONE;
      }
      break;
    }

    redpine_tune_freq(bind.freq_offset);

    timer_timeout_set_ms(50);
    done = 0;

    while (!timer_timeout() && !done) {
      if (!radio_received_packet()) {
        radio_switch_antenna();
        continue;
      }

      if (!redpine_handle_overflows()) {
        radio_reset_packet();
        radio_enable_rx();
        radio_strobe(RFST_SRX);
      }

      if (!REDPINE_VALID_PACKET_BIND(packet)) {
        continue;
      }

      debug_print("B");

      received_bind_packet = 1;
      done = 1;

      fscal0_min = min(fscal0_min, bind.freq_offset);
      fscal0_max = max(fscal0_max, bind.freq_offset);

      packet[0] = 0x00;
    }
    if (!done) {
      debug_print("-");
    }
  }

  // set offset to what we found out to be the best:
  int8_t fscal0_calc = (fscal0_max + fscal0_min) / 2;
  bind.freq_offset = fscal0_calc;

  redpine_tune_freq(bind.freq_offset);
}

void redpine_fetch_txid() {
  redpine_enter_rxmode(0);

  bind.txid[0] = 0;
  bind.txid[1] = 0;

  timer_timeout_set_ms(9 * 3 + 1);

  uint16_t hopdata_received = 0;
  while (hopdata_received != HOPDATA_RECEIVE_DONE) {

    if (timer_timeout()) {
      debug_print("m");
      timer_timeout_set_ms(3 * 9 + 1);

      radio_strobe(RFST_SIDLE);
      delay_ms(1);

      radio_reset_packet();
      radio_enable_rx();
      radio_strobe(RFST_SRX);
    }

    if (!radio_received_packet()) {
      radio_switch_antenna();
      continue;
    }

    if (!redpine_handle_overflows()) {
      radio_reset_packet();
      radio_enable_rx();
      radio_strobe(RFST_SRX);
    }

    if (!REDPINE_VALID_PACKET_BIND(packet)) {
      continue;
    }

    timer_timeout_set_ms(3 * 9 + 1);

    debug_print("B");
    if ((bind.txid[0] == 0) && (bind.txid[1] == 0)) {
      // no! extract this
      bind.txid[0] = packet[3];
      bind.txid[1] = packet[4];
    }

    uint8_t index = packet[5];
    if (index / 5 < MAX_BIND_PACKET_COUNT) {

      for (int i = 0; i < 5; i++) {
        if ((index + i) < HOPTABLE_SIZE) {
          bind.hop_table[index + i] = packet[6 + i];
        }
      }

      hopdata_received |= (1 << (index / 5));
    }

    packet[REDPINE_PACKET_BUFFER_SIZE - 1] = 0x00;
  }

  radio_strobe(RFST_SIDLE);
}

void redpine_calibrate() {
  redpine_tune_freq(bind.freq_offset);

  for (int i = 0; i < HOPTABLE_SIZE; i++) {
    uint8_t ch = bind.hop_table[i];
    redpine_tune_channel(ch);
    fscal1_table[i] = radio_read_reg(FSCAL1);
  }

  fscal3 = radio_read_reg(FSCAL3);
  fscal2 = radio_read_reg(FSCAL2);

  radio_strobe(RFST_SIDLE);
}

void redpine_bind() {
  debug_print("redpine_bind\r\n");
  bind.txid[0] = 0x03;
  bind.freq_offset = 0;

  redpine_tune();
  redpine_fetch_txid();
}

void redpine_init() {
  debug_print("redpine_init\r\n");

  redpine_configure();

  storage_read((uint8_t *)&bind, sizeof(bind_data));
  if (bind.txid[0] == 0x0 && bind.txid[1] == 0x0) {
    redpine_bind();
    storage_write((uint8_t *)&bind, sizeof(bind_data));
  }

  redpine_calibrate();
  delay_ms(100);
}

inline void redpine_send_update() {
  // seed crc
  RNDL = 0xFF;
  RNDL = 0xFF;

  // move rssi up
  packet[CHANNEL_START + 7] = packet[REDPINE_PACKET_SIZE];

  for (uint8_t i = 3; i < REDPINE_PACKET_SIZE; i++) {
    RNDH = packet[i];
  }

  // set crc
  packet[1] = RNDH;
  packet[2] = RNDL;

  // drop size, lqi & rssi from packet
  uart_dma_start(packet + 1, REDPINE_PACKET_SIZE - 1);
}

void redpine_main() {
  debug_print("redpine_main\r\n");

  redpine_enter_rxmode(bind.hop_table[current_channel_index]);

  timer_timeout_set_ms(500);

  uint8_t conn_lost = 1;
  uint8_t missing = 0;
  uint8_t packet_received = 0;

  uint16_t looptime = DEFAULT_PACKET_TIME_US;
  while (1) {
    if (timer_timeout()) {
      if (conn_lost) {
        // connection lost, do a full sync
        timer_timeout_set_100us(DEFAULT_PACKET_TIME_US);
      } else if (packet_received) {
        //Add 1/8 looptime jitter for packets
        timer_timeout_set_100us(looptime + looptime / 8);
      } else {
        //If you missed the last packet don't add the jitter
        timer_timeout_set_100us(looptime);
      }

      redpine_increment_channel(1);

      radio_enable_rx();
      radio_strobe(RFST_SRX);

      if (!packet_received) {
        missing++;
        led_red_on();
      }
      if (missing >= 5 && (missing % 5) == 0) {
        radio_switch_antenna();
      }
      if (missing >= 50) {
        conn_lost = 1;
      }
      if (missing >= 250) {
        radio_strobe(RFST_SIDLE);
        radio_init();
        radio_reset_packet();
        timer_timeout_set_100us(0);
        missing = 50;
        continue;
      }
      packet_received = 0;
    }

    redpine_handle_overflows();

    if (!radio_received_packet()) {
      packet_received = 0;
      continue;
    }

    led_red_off();
    radio_reset_packet();

    if (!REDPINE_VALID_PACKET(packet)) {
      packet_received = 0;
      continue;
    }

    led_green_on();

    looptime = packet[CHANNEL_START + 7];
    redpine_send_update();
    timer_timeout_set_100us(0);

    missing = 0;
    packet_received = 1;
    conn_lost = 0;

    led_green_off();
  }
}