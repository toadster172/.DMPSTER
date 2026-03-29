#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

char *strcat(char *dst, const char *src);
int strncmp(const char *s1, const char *s2, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void memset(void *dst, int c, size_t n);
uint8_t *pixelSet(uint8_t *dst, uint8_t pixel, uint8_t color, uint16_t len);

#endif
