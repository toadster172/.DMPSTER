// SPDX-License-Identifier: GPL-3.0-only
// Copyright 2026 toadster172 <toadster172@gmail.com>

#ifndef LEAPSTER_HW_H
#define LEAPSTER_HW_H

#include <stdint.h>

#define LEAPSTER_SCREEN_H 160
#define LEAPSTER_SCREEN_W 160
#define LEAPSTER_SCREEN_PITCH 240

#define BIOS_MAPPING_BASE ((void *) 0x4000'0000)
#define CART_MAPPING_BASE ((void *) 0x8000'0000)

struct AudioWord_ {
    uint16_t data;
    uint16_t padding;
} __attribute__((uncached));

typedef struct AudioWord_ AudioWord;

struct AudioLword_ {
    uint16_t high;
    uint16_t padding0;
    uint16_t low;
    uint16_t padding1;
} __attribute__((uncached));

typedef struct AudioLword_ AudioLword;

#define hw_apuCommand ((volatile __attribute__((uncached)) uint8_t *) 0x0180'2070)

#define APU_CMD_TRIGGER 0x80

#define hw_apuWaveStartPtrs ((volatile AudioLword *) 0x0180'40C4)
#define hw_apuWaveEndPtrs ((volatile AudioLword *) 0x0180'4104)
#define hw_apuVolume0 ((volatile AudioWord *) 0x0180'413C)
#define hw_apuDecayMaybe ((volatile AudioWord *) 0x0180'4184)
#define hw_apuVolume1 ((volatile AudioWord *) 0x0180'41A4)

#define CART_ABSENT_BIT (0x0400'0000)

// This is not a good name, but some of the bits may be settable by shorting pins on carts?
//   It also has a bit that definitely indicates whether or not a cart is present
#define hw_cartFlags ((volatile __attribute__((uncached)) uint32_t *) 0x1809'004)

#endif
