#ifndef REDPINE_H
#define REDPINE_H

#define REDPINE_PACKET_SIZE 11
#define REDPINE_PACKET_SIZE_W_ADDONS (REDPINE_PACKET_SIZE + 2)

#define REDPINE_PACKET_BUFFER_SIZE REDPINE_PACKET_SIZE_W_ADDONS

void redpine_init();

void redpine_main();

#endif