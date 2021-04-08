#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include "config.h"

#include "frsky.h"
#include "redpine.h"

#ifdef PROTOCOL_REDPINE
#define RADIO_RX_BUF_SIZE REDPINE_PACKET_BUFFER_SIZE
#endif

#ifdef PROTOCOL_FRSKY
#define RADIO_RX_BUF_SIZE FRSKY_PACKET_BUFFER_SIZE
#endif

#define HOPTABLE_SIZE 49
#define MAX_HOPTABLE_SIZE 50

typedef struct {
  uint8_t txid[2];
  uint8_t hop_table[MAX_HOPTABLE_SIZE];
  int8_t freq_offset;
} bind_data;

void protocol_init();
void protocol_main();

#endif