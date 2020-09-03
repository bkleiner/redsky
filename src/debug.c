#include "debug.h"

#ifdef DEBUG_OUTPUT

uint8_t _putc(char *buf, char *ptr, uint16_t buf_len, char c) {
  if ((ptr - buf + 1) >= buf_len) {
    return 0;
  }

  *ptr++ = c;
  *ptr = '\0';
  return 1;
}

static uint16_t _itoa(char *buf, char *ptr, uint16_t buf_len, int value, uint8_t radix, uint8_t unsig) {
  if (radix != 10 && radix != 16)
    return 0;

  char *start = ptr;
  uint8_t negative = 0;
  if (value < 0 && !unsig) {
    negative = 1;
    value = -value;
  }

  do {
    int digit = value % radix;
    char c = (digit < 10 ? '0' + digit : 'a' + digit - 10);
    if (!_putc(buf, ptr, buf_len, c)) {
      return ptr - buf;
    }
    ptr++;
    value /= radix;
  } while (value > 0);

  if (negative) {
    _putc(buf, ptr, buf_len, '-');
    ptr++;
  }

  uint16_t len = ptr - start;
  for (uint16_t i = 0; i < len / 2; i++) {
    char j = start[i];
    start[i] = start[len - i - 1];
    start[len - i - 1] = j;
  }
  return len;
}

uint16_t debug_vsnprintf(char *buf, uint16_t buf_len, char *fmt, va_list va) {
  char *ch = fmt;
  char *ptr = buf;

  while (*ch) {
    if ((ptr - buf + 1) >= buf_len) {
      break;
    }

    if (*ch != '%') {
      _putc(buf, ptr, buf_len, *ch);
      ptr++;
      ch++;
      continue;
    }

    switch (*++ch) {
    case 0:
      goto end;

    case 'u':
    case 'd': {
      ptr += _itoa(buf, ptr, buf_len, va_arg(va, unsigned int), 10, (*ch == 'u'));
      break;
    }

    case 'x': {
      ptr += _itoa(buf, ptr, buf_len, va_arg(va, unsigned int), 16, 1);
      break;
    }

    case 'c': {
      _putc(buf, ptr, buf_len, (char)(va_arg(va, int)));
      ptr++;
      break;
    }

    case 's': {
      char *s = va_arg(va, char *);
      while (*s) {
        if (!_putc(buf, ptr, buf_len, *s)) {
          break;
        }
        ptr++;
        s++;
      }
      break;
    }

    default:
      _putc(buf, ptr, buf_len, *ch);
      ptr++;
      break;
    }

    ch++;
  };

end:
  return ptr - buf;
}

uint16_t debug_snprintf(char *buf, uint16_t buf_len, char *fmt, ...) {
  uint8_t ret;

  va_list va;
  va_start(va, fmt);
  ret = debug_vsnprintf(buf, buf_len, fmt, va);
  va_end(va);

  return ret;
}

#endif