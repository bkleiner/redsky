#ifndef RADIO_H
#define RADIO_H

#include <stdint.h>

#include "cc25xx.h"

void radio_init();

void radio_dma_isr();
void radio_isr(void) __interrupt(RF_VECTOR);

#endif