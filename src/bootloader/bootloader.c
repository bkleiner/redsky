#include "bootloader.h"

#include "clock.h"
#include "delay.h"
#include "flash.h"
#include "led.h"
#include "uart.h"

uint8_t read_address(uint16_t *addr) {
  uint8_t checksum = 0;
  uint8_t data = 0;

  if (!uart_get(&data, 0xFFFF)) {
    return 0;
  }
  checksum ^= data;

  if (!uart_get(&data, 0xFFFF)) {
    return 0;
  }
  checksum ^= data;

  if (!uart_get(&data, 0xFFFF)) {
    return 0;
  }
  *addr = data;
  checksum ^= data;

  if (!uart_get(&data, 0xFFFF)) {
    return 0;
  }
  *addr = ((*addr) << 8) | data;
  checksum ^= data;

  if (!uart_get(&data, 0xFFFF)) {
    return 0;
  }

  if (checksum != data) {
    return 0;
  }

  return 1;
}

void bootloader() {
  bootloader_state state = STATE_IDLE;

  uint8_t cmd = 0;
  uint8_t data = 0;
  __xdata uint8_t buf[512];

  while (1) {
    led_green_toggle();

    switch (state) {
    default:
    case STATE_IDLE:
      if (uart_get(&cmd, 0xFFFF)) {
        if (cmd == CMD_INIT) {
          uart_put(RESPONSE_ACK);
        } else {
          state = STATE_CHECKSUM;
        }
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

    case (STATE_CMD + CMD_READ_MEMORY): {
      uint16_t addr = 0;
      if (!read_address(&addr)) {
        state = STATE_ERROR;
        break;
      }
      uart_put(RESPONSE_ACK);

      uint8_t len = 0;
      if (!uart_get(&len, 0xFFFF)) {
        state = STATE_ERROR;
        break;
      }

      if (!uart_get(&data, 0xFFFF)) {
        state = STATE_ERROR;
        break;
      }

      if (data != (len ^ 0xFF)) {
        state = STATE_ERROR;
        break;
      }
      uart_put(RESPONSE_ACK);

      flash_read(addr, buf, ((uint16_t)len) + 1);

      for (uint16_t i = 0; i < len + 1; i++) {
        uart_put(buf[i]);
      }
      state = STATE_IDLE;
      break;
    }

    case (STATE_CMD + CMD_WRITE_MEMORY): {
      uint16_t addr = 0;
      if (!read_address(&addr)) {
        state = STATE_ERROR;
        break;
      }
      uart_put(RESPONSE_ACK);

      uint8_t len = 0;
      if (!uart_get(&len, 0xFFFF)) {
        state = STATE_ERROR;
        break;
      }

      if (!uart_get(&data, 0xFFFF)) {
        state = STATE_ERROR;
        break;
      }

      if (data != (len ^ 0xFF)) {
        state = STATE_ERROR;
        break;
      }
      uart_put(RESPONSE_ACK);

      for (uint16_t i = 0; i < len + 1; i++) {
        if (!uart_get(&data, 0xFFFF)) {
          state = STATE_ERROR;
          break;
        }
        buf[i] = data;
      }
      if (state == STATE_ERROR) {
        break;
      }

      flash_write(addr, buf, ((uint16_t)len) + 1);
      state = STATE_IDLE;
      break;
    }

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
  clock_init();
  uart_init();
  led_red_off();

  led_green_on();
  for (uint8_t i = 0; i < 250; i++) {
    uint8_t magic = 0;
    if (uart_get(&magic, 0x1FFF) && magic == CMD_INIT) {
      uart_put(RESPONSE_ACK);
      bootloader();
    }
  }
  led_green_off();

  // jump to app
}