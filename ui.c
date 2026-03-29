// SPDX-License-Identifier: GPL-3.0-only
// Copyright 2026 toadster172 <toadster172@gmail.com>

#include "ui.h"
#include <stdarg.h>
#include "chorus.h"
#include "leapsterHW.h"

static struct {
    uint8_t *fb;
    uint16_t fgColor;
    uint16_t bgColor;
    uint8_t column;
    uint8_t row;
} g_termState;

const uint8_t font[256][16] = {
    #embed "CP437.F16"
};

static void fbPutChar(char c);

void clearLine(uint8_t y) {
    uint8_t c, r;

    c = g_termState.column;
    r = g_termState.row;

    // Yes, this is stupid. To be replaced by something less dumb later
    reposition(0, y);
    fbPrintf("                    ");
    reposition(c, r);
}

void clearLines(uint8_t y) {
    for(int i = y; i < 8; i++) {
        clearLine(i);
    }
}

void flushFB(void) {
    while(!gp->mpi->lcd->lockScreen(true)) { }

    gp->mpi->lcd->copyToScreen(g_termState.fb, 0, 160, true, NULL, true);
    gp->mpi->lcd->unlockScreen();
}

void initFB(void *fb, uint16_t fgColor, uint16_t bgColor) {
    g_termState.fb = fb;
    g_termState.fgColor = fgColor;
    g_termState.bgColor = bgColor;
    g_termState.column = 0;
    g_termState.row = 0;
};

int itoa(uint32_t n, char *str, int base) {
    int index = 0;

    do {
        uint8_t digit = n % base;
        digit += '0';

        if(digit > '9') {
            digit += 'A' - '9' - 1;
        }

        str[index] = digit;

        n /= base;
        index++;
    } while(n);

    for(int i = 0; i < index / 2; i++) {
        char tmp = str[i];
        str[i] = str[index - i - 1];
        str[index - i - 1] = tmp;
    }

    str[index] = '\0';

    return index;
}

static void writeFormatInt(const char *str, int strLen, int padLen, bool padZero) {
    if(padLen != -1) {
        for(int i = 0; i < padLen - strLen; i++) {
            if(padZero) {
                fbPutChar('0');
            } else {
                fbPutChar(' ');
            }
        }
    }

    for(int i = 0; i < strLen; i++) {
        fbPutChar(str[i]);
    }
}

static void fbVprintf(const char *formatString, va_list args) {
    for(int i = 0; formatString[i] != '\0'; i++) {
        if(formatString[i] != '%') {
            fbPutChar(formatString[i]);
            continue;
        }

        if(formatString[i + 1] == '%') {
            fbPutChar('%');
            i++;
            continue;
        }

        int parseIndex = i + 1;
        int padLen = -1;
        int dataSize;
        bool padZeros = false;

        if(formatString[parseIndex] == '0') {
            padZeros = true;
            parseIndex++;
        }

        if(formatString[parseIndex] >= '1' && formatString[parseIndex] <= '9') {
            padLen = 0;

            for(; formatString[parseIndex] >= '1' && formatString[parseIndex] <= '9'; parseIndex++) {
                padLen *= 10;
                padLen += formatString[parseIndex] - '0';
            }
        }

        if(formatString[parseIndex] == 'h') {
            parseIndex++;

            if(formatString[parseIndex] == 'h') {
                dataSize = 8;
                parseIndex++;
            } else {
                dataSize = 16;
            }
        } else if(formatString[parseIndex] == 'l') {
            parseIndex++;

            dataSize = 32;
        } else {
            dataSize = 32;
        }

        switch(formatString[parseIndex]) {
            case 'd':
            case 'i': {
                char str[30];

                uint32_t n = va_arg(args, unsigned int);

                if(dataSize < 32) {
                    n &= (1 << dataSize) - 1;
                }

                int len = itoa(n, str, 10);

                writeFormatInt(str, len, padLen, padZeros);

                break;
            }
            case 'X':
            case 'x': {
                char str[30];

                uint32_t n = va_arg(args, unsigned int);

                if(dataSize < 32) {
                    n &= (1 << dataSize) - 1;
                }

                int len = itoa(n, str, 16);

                writeFormatInt(str, len, padLen, padZeros);

                break;
            }
            case 'c': {
                char c = (va_arg(args, int)) & 0xFF;

                fbPutChar(c);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);

                for(int j = 0; j < (uint32_t) padLen && s[j] != '\0'; j++) {
                    fbPutChar(s[j]);
                }

                break;
            }
        }

        i = parseIndex;
    }
}

void fbPrintf(const char *formatString, ...) {
    va_list args;

    va_start(args, formatString);
    fbVprintf(formatString, args);
    va_end(args);
}

void reposition(uint8_t x, uint8_t y) {
    g_termState.column = x;
    g_termState.row = y;
}

void setFgColor(uint16_t c) {
    g_termState.fgColor = c;
}

static void fbPutChar(char c) {
    if(c == '\n') {
        g_termState.column = 0;
        g_termState.row = g_termState.row == 7 ? 0 : g_termState.row + 1;
        return;
    }

    uint16_t bg = g_termState.bgColor & 0x0FFF;
    uint16_t fg = g_termState.fgColor & 0x0FFF;

    for (int i = 0; i < 16; i++) {
        uint8_t *scanlinePtr = g_termState.fb + ((g_termState.row * 16) + i) * LEAPSTER_SCREEN_PITCH +
                                                g_termState.column * 8 * 3 / 2;
        uint8_t charData = font[c][i];

        for (int j = 0; j < 4; j++) {
            // GB R|R GB. Yuck
            scanlinePtr[j * 3] = charData & 0x80 ? (fg & 0xFF) : (bg & 0xFF);
            scanlinePtr[j * 3 + 1] = charData & 0x80 ? ((fg >> 4) & 0xF0) : ((bg >> 4) & 0xF0);
            scanlinePtr[j * 3 + 1] |= charData & 0x40 ? (fg >> 8) : (bg >> 8);
            scanlinePtr[j * 3 + 2] = charData & 0x40 ? (fg & 0xFF) : (bg & 0xFF);

            charData <<= 2;
        }
    }

    if(g_termState.column == 19) {
        g_termState.column = 0;
        g_termState.row = g_termState.row == 7 ? 0 : g_termState.row + 1;
    } else {
        g_termState.column++;
    }
}

// static void fbPrint(uint8_t *framebuffer, char *s, uint8_t x, uint8_t y) {
//     for (int i = 0; s[i] != '\0'; i++) {
//         if (s[i] == '\n') {
//             x = 0;
//             y++;
//
//             if (y == 10) {
//                 return;
//             }
//
//             continue;
//         }
//
//         fbPutChar(framebuffer, s[i], x, y);
//
//         if (x == 19) {
//             x = 0;
//             y++;
//
//             if (y == 10) {
//                 return;
//             }
//         } else {
//             x++;
//         }
//     }
// }
