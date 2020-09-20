#ifndef REDPINE_H
#define REDPINE_H

#define REDPINE_PACKET_SIZE 11
#define REDPINE_PACKET_SIZE_W_ADDONS (REDPINE_PACKET_SIZE + 2)

#define REDPINE_PACKET_BUFFER_SIZE REDPINE_PACKET_SIZE_W_ADDONS

/* Repine Packet layout
  0x00: PACKET_SIZE - 1
  0x01: ADDR_0
  0x02: ADDR_1
  0x03-0x09: CHANNEL_DATA
  0x0A: LOOPTIME
*/

void redpine_init();
void redpine_main();

#endif