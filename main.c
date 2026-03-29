// SPDX-License-Identifier: GPL-3.0-only
// Copyright 2025, 2026 toadster172 <toadster172@gmail.com>

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "chorus.h"
#include "leapsterHW.h"
#include "string.h"
#include "ui.h"

const char lfCopyright[] = "Copyright LeapFrog     ";

uint32_t g_ProcessedButtons;
uint8_t g_readBuf[0x8000] __attribute__ ((aligned (4)));

#define NEW_PRESSES(INPUT_STATE) ((INPUT_STATE) & (~g_ProcessedButtons))

static uint32_t awaitInput(uint32_t button) {
    struct inputState s;

    for(;;) {
        flushFB(); // This is a hack done because sometimes flushFB doesn't actually send the FB :\

        gp->mpi->button->getInputState(&s);

        uint32_t new = NEW_PRESSES(s.pressedButtons);
        g_ProcessedButtons = s.pressedButtons;

        if(new & button) {
            return new;
        }
    }
}

static void reportError(const char *msg, uint32_t x, uint32_t y) {
    reposition(x, y);
    setFgColor(0xF00);
    fbPrintf(msg);
    flushFB();
    awaitInput(BUTTON_A);
    setFgColor(0xFFF);
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
static uint32_t bitCeil(uint32_t n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

uint32_t getDevSize(void *base) {
    struct ribHeader *ptr = base + 0x100;

    // Check device validity
    if(strncmp(ptr->copyright, lfCopyright, 23)) {
        return 0;
    }

    uint32_t maxAddr = ptr->fullChecksum > ptr->sparseChecksum ?
                       (uint32_t) ptr->fullChecksum : (uint32_t) ptr->sparseChecksum;

    return bitCeil(maxAddr - ((uint32_t) base));
}

static bool writeFile(const char *filename, void *ptr, uint32_t size) {
    chorusFile *f = gp->mpi->fat32->fopen(filename, "wb");

    reposition(0, 1);

    if(f == NULL) {
        reportError("Failed file open\nA: Return\n", 0, 1);
        return false;
    }

    fbPrintf("Opened out file!\n");

    for(int i = 0; i < size / 0x8000; i++) {
        reposition(0, 2);
        fbPrintf("Dumping... %02i%%", (i * 100) / (size / 0x8000));
        flushFB();

        size_t s = gp->mpi->fat32->fwrite(ptr + (i * 0x8000), 1, 0x8000, f);

        if(s != 0x8000) {
            gp->mpi->fat32->fclose(f);
            clearLines(1);
            reportError("Failed write!\nA: Return\n", 0, 1);

            return false;
        }
    }

    gp->mpi->fat32->fclose(f);

    return true;
}

static bool readbackFile(const char *filename, void *ptr, uint32_t size) {
    struct ribHeader header;

    clearLines(1);
    reposition(0, 1);
    fbPrintf("Reading back file\n");
    flushFB();

    chorusFile *f = gp->mpi->fat32->fopen(filename, "rb");

    if(f == NULL) {
        reportError("Failed file reopen\nA: Return\n", 0, 2);
        goto fatalError;
    }

    gp->mpi->fat32->fseek(f, 0x100, CHORUS_SEEK_SET);

    if(gp->mpi->fat32->fread(&header, sizeof(header), 1, f) != 1) {
        reportError("Failed reread\nA: Return\n", 0, 2);
        goto fatalErrorFclose;
    }

    size_t devSize = ((size_t) header.deviceEnd) - ((size_t) header.deviceStart) + 4;
    size_t checksumOffset = ((size_t) header.fullChecksum) - ((size_t) header.deviceStart);

    uint32_t fullChecksum;
    gp->mpi->fat32->fseek(f, checksumOffset, CHORUS_SEEK_SET);

    if(gp->mpi->fat32->fread(&fullChecksum, sizeof(fullChecksum), 1, f) != 1) {
        reportError("Failed reread\nA: Return\n", 0, 2);
        goto fatalErrorFclose;
    }

    gp->mpi->fat32->fseek(f, 0, CHORUS_SEEK_SET);

    uint32_t currChecksum = 0;
    uint32_t summedLwords = 0;

    for(int i = 0; i < size / 0x8000; i++) {
        reposition(0, 2);
        fbPrintf("Rereading... %02i%%", (i * 100) / (size / 0x8000));
        flushFB();
        size_t res = gp->mpi->fat32->fread(g_readBuf, 1, 0x8000, f);

        if(memcmp(ptr + i * 0x8000, g_readBuf, 0x8000)) {
            reportError("Reread mismatch!\nA: Return\n", 0, 4);
            goto fatalErrorFclose;
        }

        for(int j = 0; j < 0x2000 && summedLwords < (devSize / 4); j++, summedLwords++) {
            currChecksum += ((uint32_t *) g_readBuf)[j];
        }
    }

    gp->mpi->fat32->fclose(f);

    if(currChecksum != fullChecksum) {
        reposition(0, 3);
        fbPrintf("Checksum mismatch!\nA: Delete\nB: Rename\n");
        uint32_t in = awaitInput(BUTTON_A | BUTTON_B);

        if(in & BUTTON_A) {
            goto fatalError;
        } else {
            char buf[50];

            buf[0] = '\0';
            strcat(buf, "BADDMP_");
            strcat(buf, filename);
            gp->mpi->fat32->rename(filename, buf);
        }
    } else {
        clearLines(1);
        reposition(0, 1);
        fbPrintf("Dump verified!\nA: Return\n");
        flushFB();
        awaitInput(BUTTON_A);
    }

    return true;

fatalErrorFclose:
    gp->mpi->fat32->fclose(f);
fatalError:
    gp->mpi->fat32->fdelete(filename);

    return false;
}

static void tryDump(char *baseName, void *ptr, size_t size) {
    char filenameBuf[40];
    char numberBuf[10];

    clearLines(1);
    flushFB();

    uint32_t checksum = *((struct ribHeader *) (ptr + 0x100))->fullChecksum;
    itoa(checksum, numberBuf, 16);

    filenameBuf[0] = '\0';
    strcat(filenameBuf, baseName);
    strcat(filenameBuf, "_0x");
    strcat(filenameBuf, numberBuf);
    strcat(filenameBuf, ".bin");

    if(writeFile(filenameBuf, ptr, size)) {
        readbackFile(filenameBuf, ptr, size);
    }
}

extern uint8_t *__start_BSS;
extern uint8_t *__stop_BSS;

void entry(void) {
    memset(__start_BSS, 0, ((uint32_t) &__stop_BSS) - ((uint32_t) &__start_BSS));

    uint8_t *framebuffer = gp->mpi->kernel->mallocZero(LEAPSTER_SCREEN_H * LEAPSTER_SCREEN_PITCH);
    initFB(framebuffer, 0xFFF, 0x000);

    gp->mpi->fat32->chdir("B:\\Leapster");

    fbPrintf("      .DMPSTER\n");
    flushFB();

    uint32_t biosSize = getDevSize(BIOS_MAPPING_BASE);
    uint32_t cartSize = getDevSize(CART_MAPPING_BASE);

    for(;;) {
outer:
        clearLines(1);
        reposition(0, 1);

        setFgColor(biosSize != 0 ? 0xFFF : 0x444);
        fbPrintf("\nA: Dump BIOS\n");

        setFgColor(cartSize != 0 ? 0xFFF : 0x444);
        fbPrintf("B: Dump Cart\n");
        setFgColor(0xFFF);
        flushFB();

        for(;;) {
            uint32_t new = awaitInput(BUTTON_A | BUTTON_B);

            if((new & BUTTON_A) && biosSize != 0) {
                tryDump("BIOS", BIOS_MAPPING_BASE, biosSize);
                goto outer;
            }

            if((new & BUTTON_B) && cartSize != 0) {
                tryDump("CART", CART_MAPPING_BASE, cartSize);
                goto outer;
            }
        }
    }
}

static bool hbActiveFlag;

static bool hbInit(void) {
    hbActiveFlag = true;
    entry();
    return true;
}

static bool hbDeInit(void) {
    hbActiveFlag = false;
    return true;
}

static bool hbIsActive(void) {
    return false;
}

static uint32_t hbGetVersion(void) {
    return 0x100;
}

const char hbMPIName[] = "HomebrewMPI";
const char hbMPIDesc[] = "Hack to get native code running";

static const char *hbGetName(void) {
    return hbMPIName;
}

static const char *hbGetDesc(void) {
    return hbMPIDesc;
}

const struct moduleInterface hbMPI = {
    .init = hbInit,
    .deInit = hbDeInit,
    .isActive = hbIsActive,
    .getVersion0 = hbGetVersion,
    .getVersion1 = hbGetVersion,
    .getVersion2 = hbGetVersion,
    .getName = hbGetName,
    .getDesc = hbGetDesc
};
