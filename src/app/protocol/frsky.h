#ifndef FRSKY_H
#define FRSKY_H

#define FRSKY_HOPTABLE_SIZE 47

#define FRSKY_PACKET_SIZE 17
#define FRSKY_PACKET_SIZE_W_ADDONS (FRSKY_PACKET_SIZE + 3)

#define FRSKY_PACKET_BUFFER_SIZE FRSKY_PACKET_SIZE_W_ADDONS

void frsky_init();
void frsky_main();

#endif