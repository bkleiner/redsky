#include "protocol/protocol.h"

#include "driver.h"

#include "frsky.h"
#include "redpine.h"

EXT_MEMORY uint8_t current_channel_index = 0;

EXT_MEMORY uint8_t fscal1_table[HOPTABLE_SIZE];
EXT_MEMORY uint8_t fscal2;
EXT_MEMORY uint8_t fscal3;

bind_data EXT_MEMORY bind = {
    .txid = {0xFF, 0xFF},
};

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