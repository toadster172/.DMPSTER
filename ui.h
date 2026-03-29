#ifndef UI_H
#define UI_H

#include <stdint.h>

void clearLine(uint8_t y);
void clearLines(uint8_t y);
void fbPrintf(const char *formatString, ...);
void flushFB(void);
void initFB(void *fb, uint16_t fgColor, uint16_t bgColor);
int itoa(uint32_t n, char *str, int base); // Should be moved to a more appropriate header when one is made
void reposition(uint8_t x, uint8_t y);
void setFgColor(uint16_t color);

#endif
