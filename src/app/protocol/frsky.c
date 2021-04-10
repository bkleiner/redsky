#include "frsky.h"

#include <string.h>

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

#define DEFAULT_PACKET_TIME 90
#define TELEMETRY_TIME 20
#define LOST_PACKET_TIME 5000
#define PACKET_JITTER_TIME 20

#define MAX_BIND_PACKET_COUNT 10
#define HOPDATA_RECEIVE_DONE ((1 << (MAX_BIND_PACKET_COUNT)) - 1)

#define FRSKY_VALID_FRAMELENGTH(_b) (_b[0] == 0x11)
#define FRSKY_VALID_CRC(_b) (_b[19] & 0x80)
#define FRSKY_VALID_TXID(_b) ((_b[1] == bind.txid[0]) && (_b[2] == bind.txid[1]))

#define FRSKY_VALID_PACKET_BIND(_b) (FRSKY_VALID_FRAMELENGTH(_b) && FRSKY_VALID_CRC(_b) && (_b[2] == 0x01))
#define FRSKY_VALID_PACKET(_b) (FRSKY_VALID_FRAMELENGTH(_b) && FRSKY_VALID_CRC(_b) && FRSKY_VALID_TXID(_b))

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
  radio_write_reg(PKTCTRL1, 0x04);
  radio_write_reg(PKTCTRL0, 0x05);
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

  radio_write_reg(AGCCTRL2, 0b01000011);
  radio_write_reg(AGCCTRL1, 0b01000000);
  radio_write_reg(AGCCTRL0, 0b10010001);

  radio_write_reg(FREND1, 0x56);
  radio_write_reg(FREND0, 0x10);

  radio_write_reg(FSCAL3, 0xA9);
  radio_write_reg(FSCAL2, 0x0A);
  radio_write_reg(FSCAL1, 0x00);
  radio_write_reg(FSCAL0, 0x11);

  //radio_write_reg(FSTEST, 0x59);

  radio_write_reg(TEST2, 0x88);
  radio_write_reg(TEST1, 0x31);
  radio_write_reg(TEST0, 0x0B);

  radio_write_reg(ADDR, 0x00);
}

static void frsky_tune() {
  debug_print("frsky_tune\r\n");

  bind.freq_offset = 0;

  protocol_enter_rxmode(0);

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

    protocol_tune_freq(bind.freq_offset);

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

  protocol_tune_freq(bind.freq_offset);
}

static void frsky_fetch_txid() {
  protocol_enter_rxmode(0);

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

static void frsky_bind() {
  debug_print("frsky_bind\r\n");
  bind.txid[0] = 0x03;
  bind.freq_offset = 0;

  protocol_set_address();

  frsky_tune();
  frsky_fetch_txid();
}

static void frsky_send_update(uint8_t packet_lost) {

  static EXT_MEMORY uint16_t channel_data[8];

  channel_data[0] = (uint16_t)((packet[10] & 0x0F) << 8 | packet[6]) - 1260;
  channel_data[1] = (uint16_t)((packet[10] & 0xF0) << 4 | packet[7]) - 1260;
  channel_data[2] = (uint16_t)((packet[11] & 0x0F) << 8 | packet[8]) - 1260;
  channel_data[3] = (uint16_t)((packet[11] & 0xF0) << 4 | packet[9]) - 1260;
  channel_data[4] = (uint16_t)((packet[16] & 0x0F) << 8 | packet[12]) - 1260;
  channel_data[5] = (uint16_t)((packet[16] & 0xF0) << 4 | packet[13]) - 1260;
  channel_data[6] = (uint16_t)((packet[17] & 0x0F) << 8 | packet[14]) - 1260;
  channel_data[7] = (uint16_t)((packet[17] & 0xF0) << 4 | packet[15]) - 1260;

#define SBUS_SIZE 25
#define SBUS_SYNC 0x0F
#define SBUS_FLAG_FRAME_LOST (1 << 2)
#define SBUS_FLAG_FAILSAFE_ACTIVE (1 << 3)

  static EXT_MEMORY uint8_t sbus_data[SBUS_SIZE];

  memset(sbus_data, 0, SBUS_SIZE);

  sbus_data[0] = SBUS_SYNC;

  sbus_data[1] = channel_data[0];
  sbus_data[2] = (channel_data[1] << 3) | channel_data[0] >> 8;
  sbus_data[3] = (channel_data[1] >> 5) | (channel_data[2] << 6);
  sbus_data[4] = (channel_data[2] >> 2) & 0xFF;
  sbus_data[5] = (channel_data[2] >> 10) | (channel_data[3] << 1);
  sbus_data[6] = (channel_data[3] >> 7) | (channel_data[4] << 4);
  sbus_data[7] = (channel_data[4] >> 4) | (channel_data[5] << 7);
  sbus_data[8] = (channel_data[5] >> 1) & 0xFF;
  sbus_data[9] = (channel_data[5] >> 9) | (channel_data[6] << 2);
  sbus_data[10] = (channel_data[6] >> 6) | (channel_data[7] << 5);
  sbus_data[11] = (channel_data[7] >> 3) & 0xFF;

  sbus_data[23] = 0;

  if (packet_lost) {
    sbus_data[23] |= SBUS_FLAG_FRAME_LOST;
  }

  sbus_data[24] = 0;

  uart_dma_start(sbus_data, SBUS_SIZE);
}

#ifdef USE_TELEMETRY
static void frsky_send_telemetry(uint8_t id) {
  radio_strobe(RFST_SFRX);

  radio_enter_tx();

  memset((uint8_t *)packet, 0, FRSKY_PACKET_BUFFER_SIZE);

  packet[0] = 0x11; // length of byte (always 0x11 = 17 bytes)
  packet[1] = bind.txid[0];
  packet[2] = bind.txid[1];
  packet[3] = 0;   // ADC channels
  packet[4] = 0;   // ADC channels
  packet[5] = 100; // RSSI

  // no hub telemetry for now
  packet[6] = 0; // size
  packet[7] = id;

  radio_strobe(RFST_SIDLE);
  radio_transmit((uint8_t *)packet, FRSKY_PACKET_BUFFER_SIZE);

  radio_enable_rx();
}
#endif

void frsky_init() {
  debug_print("frsky_init\r\n");

  frsky_configure();

  storage_read((uint8_t *)&bind, sizeof(bind_data));
  if ((bind.txid[0] == 0x0 && bind.txid[1] == 0x0) ||
      (bind.txid[0] == 0xFF && bind.txid[1] == 0xFF)) {
    frsky_bind();
    storage_write((uint8_t *)&bind, sizeof(bind_data));
  }

  protocol_set_address();
  protocol_calibrate();
  delay_ms(100);
}

void frsky_main() {
  debug_print("frsky_main\r\n");

  protocol_enter_rxmode(bind.hop_table[current_channel_index]);
  radio_strobe(RFST_SRX);

  timer_timeout_set_100us(LOST_PACKET_TIME);

  uint8_t conn_lost = 1;
  uint8_t missing = 0;
  uint8_t packet_received = 0;

  uint8_t telemetry_id = 0;
  uint8_t telemetry_start = 0;
  uint8_t telemetry_sending = 0;

  while (1) {
    if (timer_timeout()) {
      if (conn_lost) {
        // connection lost, do a full sync
        timer_timeout_set_100us(LOST_PACKET_TIME);
      } else if (telemetry_start) {
        // we already waited 2ms down below
        timer_timeout_set_100us(DEFAULT_PACKET_TIME - TELEMETRY_TIME);
      } else if (packet_received) {
        // we got a packet, a litte bit of jitter
        timer_timeout_set_100us(DEFAULT_PACKET_TIME + PACKET_JITTER_TIME);
      } else {
        timer_timeout_set_100us(DEFAULT_PACKET_TIME);
      }

      protocol_increment_channel(1);

      if (telemetry_sending) {
        frsky_send_update(0);
        telemetry_sending = 0;
      }

      if (telemetry_start) {
#ifdef USE_TELEMETRY
        frsky_send_telemetry(telemetry_id);
#else
        radio_strobe(RFST_SIDLE);
        while (radio_read_reg(MARCSTATE) != 0x01)
          ;

        radio_reset_packet();
#endif
        telemetry_start = 0;
        telemetry_sending = 1;
      } else {
        radio_enable_rx();
        radio_strobe(RFST_SRX);
      }

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
      /*
      if (missing >= 250) {
        radio_strobe(RFST_SIDLE);
        //radio_init();
        radio_reset_packet();
        timer_timeout_set_100us(0);
        missing = 50;
        continue;
      }
      */

      radio_handle_overflows();
    }

    if (telemetry_sending || telemetry_start) {
      packet_received = 1;
      radio_reset_packet();
      continue;
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

    if ((packet[3] % 4) == 2) {
      // next frame is a telemetry frame
      telemetry_start = 1;
    }
    telemetry_id = packet[4];

    led_green_on();

    frsky_send_update(0);
    if (telemetry_start) {
      // telemetry must be sent ~2ms after rx
      timer_timeout_set_100us(TELEMETRY_TIME);
    } else {
      timer_timeout_set_100us(0);
    }

    packet[19] = 0;

    missing = 0;
    packet_received = 1;
    conn_lost = 0;
  }
}