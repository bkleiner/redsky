#include "bootloader.h"

#include "clock.h"
#include "delay.h"
#include "led.h"
#include "uart.h"

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

void bootloader() {
  uart_put(0x7F);

  bootloader_state state = STATE_IDLE;

  uint8_t cmd = 0;
  uint8_t data = 0;

  while (1) {
    led_green_toggle();

    switch (state) {
    default:
    case STATE_IDLE:
      if (uart_get(&cmd, 0xFFFF)) {
        state = STATE_CHECKSUM;
      }
      break;
    case STATE_CHECKSUM: {
      if (!uart_get(&data, 0xFFFF)) {
        state = STATE_ERROR;
        break;
      }

      // check command checksum (inverted)
      // NOTE: ~x seems to be calculated in uint16_t !
      if (data != (cmd ^ 0xFF)) {
        state = STATE_ERROR;
        break;
      }

      switch (cmd) {
      case CMD_GET:
      case CMD_GET_VERSION:
      case CMD_GET_ID:
      case CMD_READ_MEMORY:
      case CMD_GO:
      case CMD_WRITE_MEMORY:
      case CMD_ERASE:
        // send ACK and continue with command handler
        uart_put(RESPONSE_ACK);
        state = STATE_CMD + cmd;
        break;

      default:
        state = STATE_ERROR;
        break;
      }
      break;
    }

    case (STATE_CMD + CMD_GET):
      uart_put(7);
      uart_put(BOOTLOADER_VERSION);

      uart_put(CMD_GET);
      uart_put(CMD_GET_VERSION);
      uart_put(CMD_GET_ID);
      uart_put(CMD_READ_MEMORY);
      uart_put(CMD_GO);
      uart_put(CMD_WRITE_MEMORY);
      uart_put(CMD_ERASE);

      uart_put(RESPONSE_ACK);

      state = STATE_IDLE;
      break;

    case (STATE_CMD + CMD_GET_VERSION):
      uart_put(BOOTLOADER_VERSION);

      uart_put(0x00);
      uart_put(0x00);
      uart_put(RESPONSE_ACK);

      state = STATE_IDLE;
      break;

    case (STATE_CMD + CMD_GET_ID):
      uart_put(1);

      uart_put(BOOTLOADER_DEVICE_ID >> 8);
      uart_put(BOOTLOADER_DEVICE_ID & 0xFF);

      uart_put(RESPONSE_ACK);

      state = STATE_IDLE;
      break;

    case STATE_ERROR:
      uart_put(RESPONSE_NACK);
      state = STATE_IDLE;
      break;
    }
  }
}

void main() {
  led_init();

  led_red_on();
  clock_init_fast();
  uart_init();
  led_red_off();

  delay_ms(250);
}