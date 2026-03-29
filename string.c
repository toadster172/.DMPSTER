// SPDX-License-Identifier: GPL-3.0-only
// Copyright 2026 toadster172 <toadster172@gmail.com>

#include <stddef.h>
#include <stdint.h>
#include "string.h"

char *strcat(char *dst, const char *src) {
    char *orgDst = dst;

    while(*(dst++) != '\0') { }

    dst--;

    while((*(dst++) = *(src++)) != '\0') { }

    return orgDst;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for(int i = 0; i < n; i++) {
        if(*s1 != *s2 || *s1 == '\0') {
            return *s1 - *s2;
        }

        s1++;
        s2++;
    }

    return 0;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *s1_8 = s1;
    const uint8_t *s2_8 = s2;

    for(int i = 0; i < n; i++) {
        if(*s1_8 != *s2_8) {
            return *s1_8 - *s2_8;
        }

        s1_8++;
        s2_8++;
    }

    return 0;
}

// I wrote an assembly implementation of this for fun, only for it to fail spectacularly on hardware :(
//   The Leapster's requirements around loop hazards seem to be stricter than described in Arctangent
//   documentation. Or maybe I just missed something
void memset(void *dst, int c, size_t n) {
    uint8_t *dstByte = dst;
    uint8_t cByte = (uint8_t) c;
    uint32_t cWord = (cByte << 8) | cByte;
    cWord = (cWord << 16) | cWord;

    uint8_t alignDistance = (4 - (((uint32_t) dst) & 0x03)) & 0x03;

    if(alignDistance > n) {
        alignDistance = n;
    }

    switch(alignDistance) {
        case 3: *(dstByte++) = cByte;
        case 2: *(dstByte++) = cByte;
        case 1: *(dstByte++) = cByte;
    }

    n -= alignDistance;

    uint32_t *dstWord = (uint32_t *) dstByte;

    for(int i = 0; i < (n >> 2); i++) {
        *(dstWord++) = cWord;
    }

    dstByte = (uint8_t *) dstWord;

    n &= 0x03;

    switch(n) {
        case 0x03: *(dstByte++) = cByte;
        case 0x02: *(dstByte++) = cByte;
        case 0x01: *(dstByte++) = cByte;
    }
}

// It took an embarrassingly long time to debug all the issues from this function
//   I hate 12-bit color
uint8_t *pixelSet(uint8_t *dst, uint8_t pixel, uint8_t color, uint16_t len) {
    uint8_t *pixelDst = dst;

    if(pixel & 0x01) {
        *(pixelDst++) |= color & 0x0F;
        *(pixelDst++) = color;

        len--;

        if(len == 0) {
            return pixelDst;
        }
    }

    memset(pixelDst, color, (len >> 1) * 3);
    pixelDst += (len >> 1) * 3;

    if(len & 1) {
        *(pixelDst++) = color;
        *(pixelDst) = color & 0xF0;
    }

    return pixelDst;
}
