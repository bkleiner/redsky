#ifndef BOOTLOADER_H
#define BOOTLOADER_H

// version 1.0
#define BOOTLOADER_VERSION 0x10
#define BOOTLOADER_DEVICE_ID 0x0410

typedef enum {
  CMD_INIT = 0x7F,
  CMD_GET = 0x00,
  CMD_GET_VERSION = 0x01,
  CMD_GET_ID = 0x02,
  CMD_READ_MEMORY = 0x11,
  CMD_GO = 0x21,
  CMD_WRITE_MEMORY = 0x31,
  CMD_ERASE = 0x43,
} bootloader_cmd;

typedef enum {
  RESPONSE_ACK = 0x79,
  RESPONSE_NACK = 0x1F,
} bootloader_response;

typedef enum {
  STATE_IDLE,
  STATE_CHECKSUM,
  STATE_CMD,
  STATE_ERROR = 0xFF
} bootloader_state;

#endif