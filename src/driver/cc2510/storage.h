#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

void storage_init();
void storage_read(uint8_t *buf, uint16_t len);
void storage_write(uint8_t *buf, uint16_t len);

#endif