#include "string.h"

uint16_t string_len(const char *str) {
  uint16_t len = 0;
  while (*str++)
    len++;
  return len;
}

void reverse(char s[]) {
  int c, i, j;

  for (i = 0, j = string_len(s) - 1; i < j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}

uint8_t itoa(uint32_t n, char s[]) {
  int i = 0;
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  s[i] = '\0';
  reverse(s);

  return i;
}
