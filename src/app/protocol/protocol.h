#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include "config.h"

#include "frsky.h"
#include "redpine.h"

#ifdef PROTOCOL_REDPINE
#define HOPTABLE_SIZE REDPINE_HOPTABLE_SIZE
#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE
#endif

#ifdef PROTOCOL_FRSKY
#define HOPTABLE_SIZE FRSKY_HOPTABLE_SIZE
#define RADIO_RX_BUF_SIZE FRSKY_PACKET_BUFFER_SIZE
#endif

#define MAX_HOPTABLE_SIZE 50

typedef struct {
  uint8_t txid[2];
  uint8_t hop_table[MAX_HOPTABLE_SIZE];
  int8_t freq_offset;
} bind_data;

void protocol_init();
void protocol_main();

void protocol_tune_freq(int8_t freq);
void protocol_tune_channel(uint8_t ch);
void protocol_enter_rxmode(uint8_t channel);
void protocol_set_channel(uint8_t hop_index);
void protocol_increment_channel(int8_t cnt);
void protocol_calibrate();
void protocol_set_address();

#endif