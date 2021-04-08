#include "frsky.h"

#include "crc.h"
#include "debug.h"
#include "delay.h"
#include "driver.h"
#include "led.h"
#include "protocol.h"
#include "radio.h"
#include "storage.h"
#include "timer.h"
#include "util.h"

#define MAX_BIND_PACKET_COUNT 10

#define DEFAULT_PACKET_TIME_US 50000

#define CHANNEL_START 3

#define HOPDATA_RECEIVE_DONE ((1 << (MAX_BIND_PACKET_COUNT)) - 1)

#define FRSKY_VALID_PACKET_CRC(_b) (_b[FRSKY_PACKET_SIZE_W_ADDONS - 1] & (1 << 7))
#define FRSKY_VALID_PACKET_BIND(_b) ((_b[2] == 0x01) && FRSKY_VALID_PACKET_CRC(_b))

#define FRSKY_VALID_TXID(_b) ((_b[1] == bind.txid[0]) && (_b[2] == bind.txid[1]))
#define FRSKY_VALID_PACKET(_b) ((_b[0] == 10) && FRSKY_VALID_TXID(_b) && FRSKY_VALID_PACKET_CRC(_b))

typedef enum {
  TUNE_INIT,
  TUNE_FAST_SEARCH,
  TUNE_SLOW_SEARCH,
  TUNE_DONE
} tune_state;

extern volatile EXT_MEMORY uint8_t packet[RADIO_RX_BUF_SIZE];

extern EXT_MEMORY uint8_t current_channel_index;

extern EXT_MEMORY uint8_t fscal1_table[HOPTABLE_SIZE];
extern EXT_MEMORY uint8_t fscal2;
extern EXT_MEMORY uint8_t fscal3;

extern bind_data EXT_MEMORY bind;

static void frsky_configure() {
  debug_print("frsky_configure\r\n");

  radio_io_config();

  radio_write_reg(PKTLEN, FRSKY_PACKET_SIZE);
  radio_write_reg(PKTCTRL1, 0x0C);
  radio_write_reg(PKTCTRL0, 0x04);
  radio_write_reg(PA_TABLE0, 0xFF);

  radio_write_reg(FSCTRL1, 0x08);
  radio_write_reg(FSCTRL0, 0x00);

  radio_write_reg(FREQ2, 0x5C);
  radio_write_reg(FREQ1, 0x76);
  radio_write_reg(FREQ0, 0x27);

  radio_write_reg(MDMCFG4, 0xAA);
  radio_write_reg(MDMCFG3, 0x39);
  radio_write_reg(MDMCFG2, 0x11);
  radio_write_reg(MDMCFG1, 0x23);
  radio_write_reg(MDMCFG0, 0x7A);

  radio_write_reg(DEVIATN, 0x42);

  radio_write_reg(MCSM1, 0x0F);
  radio_write_reg(MCSM0, 0x18);

  radio_write_reg(FOCCFG, 0x16);
  radio_write_reg(BSCFG, 0x6C);

  radio_write_reg(AGCCTRL2, 0x03);
  radio_write_reg(AGCCTRL1, 0x40);
  radio_write_reg(AGCCTRL0, 0x91);

  radio_write_reg(FREND1, 0x56);
  radio_write_reg(FREND0, 0x10);

  radio_write_reg(FSCAL3, 0xA9);
  radio_write_reg(FSCAL2, 0x05);
  radio_write_reg(FSCAL1, 0x00);
  radio_write_reg(FSCAL0, 0x11);

  //radio_write_reg(FSTEST, 0x59);

  radio_write_reg(TEST2, 0x88);
  radio_write_reg(TEST1, 0x31);
  radio_write_reg(TEST0, 0x0B);

  radio_write_reg(ADDR, 0x00);
}

static void frsky_tune_freq(int8_t freq) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(FSCTRL0, freq);
  delay_ms(1);
  radio_strobe(RFST_SRX);
}

static void frsky_tune_channel(uint8_t ch) {
  radio_strobe(RFST_SIDLE);
  radio_write_reg(CHANNR, ch);
  radio_strobe(RFST_SCAL);

  while (radio_read_reg(MARCSTATE) != 0x01)
    ;
}

static void frsky_enter_rxmode(uint8_t channel) {
  radio_strobe(RFST_SIDLE);

  frsky_tune_channel(channel);
  radio_enable_rx();

  radio_strobe(RFST_SRX);
}

static void frsky_set_channel(uint8_t hop_index) {
  uint8_t ch = bind.hop_table[hop_index];

  radio_strobe(RFST_SIDLE);

  radio_write_reg(FSCAL3, fscal3);
  radio_write_reg(FSCAL2, fscal2);
  radio_write_reg(FSCAL1, fscal1_table[hop_index]);

  radio_write_reg(CHANNR, ch);
}

static void frsky_increment_channel(int8_t cnt) {
  int8_t next = current_channel_index + cnt;

  // convert to a safe unsigned number:
  if (next < 0) {
    next += HOPTABLE_SIZE;
  }
  if (next >= HOPTABLE_SIZE) {
    next -= HOPTABLE_SIZE;
  }

  current_channel_index = next;
  frsky_set_channel(current_channel_index);
}

static void frsky_tune() {
  debug_print("frsky_tune\r\n");

  bind.freq_offset = 0;

  radio_write_reg(FOCCFG, 0x14);

  radio_write_reg(FSCTRL0, (uint8_t)bind.freq_offset);
  radio_write_reg(PKTCTRL1, 0x0C);
  radio_write_reg(MCSM0, 0x8);

  frsky_enter_rxmode(0);

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

    default:
      break;
    }

    frsky_tune_freq(bind.freq_offset);

    timer_timeout_set_ms(50);
    done = 0;

    while (!timer_timeout() && !done) {
      if (!radio_received_packet()) {
        continue;
      }

      radio_reset_packet();
      radio_enable_rx();
      radio_strobe(RFST_SRX);

      if (!FRSKY_VALID_PACKET_BIND(packet)) {
        continue;
      }

      debug_print("B");

      received_bind_packet = 1;
      done = 1;

      fscal0_min = min(fscal0_min, bind.freq_offset);
      fscal0_max = max(fscal0_max, bind.freq_offset);

      packet[0] = 0x00;
    }

    radio_handle_overflows();

    if (!done) {
      radio_switch_antenna();
      debug_print("-");
    }
  }

  // set offset to what we found out to be the best:
  int8_t fscal0_calc = (fscal0_max + fscal0_min) / 2;
  bind.freq_offset = fscal0_calc;

  frsky_tune_freq(bind.freq_offset);
}

static void frsky_fetch_txid() {
  frsky_enter_rxmode(0);

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

      radio_handle_overflows();
      radio_switch_antenna();
    }

    if (!radio_received_packet()) {
      continue;
    }

    radio_reset_packet();
    radio_enable_rx();
    radio_strobe(RFST_SRX);

    if (!FRSKY_VALID_PACKET_BIND(packet)) {
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

    packet[FRSKY_PACKET_BUFFER_SIZE - 1] = 0x00;
  }

  radio_strobe(RFST_SIDLE);
}

static void frsky_calibrate() {
  frsky_tune_freq(bind.freq_offset);

  for (int i = 0; i < HOPTABLE_SIZE; i++) {
    uint8_t ch = bind.hop_table[i];
    frsky_tune_channel(ch);
    fscal1_table[i] = radio_read_reg(FSCAL1);
  }

  fscal3 = radio_read_reg(FSCAL3);
  fscal2 = radio_read_reg(FSCAL2);

  radio_strobe(RFST_SIDLE);
}

static void frsky_bind() {
  debug_print("frsky_bind\r\n");
  bind.txid[0] = 0x03;
  bind.freq_offset = 0;

  frsky_tune();
  frsky_fetch_txid();
}

static void frsky_send_update(uint8_t packet_lost) {
  // set magic
  packet[0] = 0x2A;
  if (packet_lost == 1) {
    packet[0] |= 0b01000000;
  }

  // move rssi up
  packet[CHANNEL_START + 7] = packet[FRSKY_PACKET_SIZE + 1] & 0x7f;

  uint16_t crc = crc_compute((uint8_t *)packet, FRSKY_PACKET_SIZE);
  WRITE_WORD(packet[1], packet[2], crc);

  // drop size, lqi & rssi from packet
  uart_dma_start((uint8_t *)packet, FRSKY_PACKET_SIZE);
}

void frsky_init() {
  debug_print("frsky_init\r\n");

  frsky_configure();

  storage_read((uint8_t *)&bind, sizeof(bind_data));
  if ((bind.txid[0] == 0x0 && bind.txid[1] == 0x0) ||
      (bind.txid[0] == 0xFF && bind.txid[1] == 0xFF)) {
    frsky_bind();
    storage_write((uint8_t *)&bind, sizeof(bind_data));
  }

  frsky_calibrate();
  delay_ms(100);
}

void frsky_main() {
  debug_print("frsky_main\r\n");

  frsky_enter_rxmode(bind.hop_table[current_channel_index]);

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

      frsky_increment_channel(1);

      radio_enable_rx();
      radio_strobe(RFST_SRX);

      if (!packet_received) {
        frsky_send_update(1);
        missing++;
        led_red_on();
        led_green_off();
      }
      packet_received = 0;

      if (missing >= 5 && (missing % 5) == 0) {
        radio_switch_antenna();
      }
      if (missing >= 50) {
        conn_lost = 1;
      }
      if (missing >= 250) {
        radio_strobe(RFST_SIDLE);
        //radio_init();
        radio_reset_packet();
        timer_timeout_set_100us(0);
        missing = 50;
        continue;
      }

      radio_handle_overflows();
    }

    if (!radio_received_packet()) {
      packet_received = 0;
      continue;
    }

    led_red_off();
    radio_reset_packet();

    if (!FRSKY_VALID_PACKET(packet)) {
      packet_received = 0;
      continue;
    }

    led_green_on();

    looptime = packet[CHANNEL_START + 7];
    frsky_send_update(0);
    timer_timeout_set_100us(0);

    missing = 0;
    packet_received = 1;
    conn_lost = 0;
  }
}