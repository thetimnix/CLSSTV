#pragma once
#include "SSTV.h"
#include "wav.h"

void encodeSC(SSTV::rgb* rgbBuffer, double lineMS, int visCode) {
    SSTV::addVisCode(visCode);

    int jpg_width = 320;
    int jpg_height = 256;

    float mspp = lineMS / (float)jpg_width;
    int pxptr = 0;

    wav::addTone(1200, 9);
    for (int y = 0; y < jpg_height; y++) {
        int tmpPxptr = pxptr;

        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[pxptr];
            wav::addTone(1500 + (3.13 * c.g), mspp);
            pxptr++;
        }
        pxptr = tmpPxptr;

        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[pxptr];
            wav::addTone(1500 + (3.13 * c.b), mspp);
            pxptr++;
        }
        pxptr = tmpPxptr;

        wav::addTone(1200, 9);
        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[pxptr];
            wav::addTone(1500 + (3.13 * c.r), mspp);
            pxptr++;
        }
    }
}

void encodeSC1(SSTV::rgb* rgbBuffer) {
    encodeSC(rgbBuffer, 138.240, 0x3C);
}

void encodeSC2(SSTV::rgb* rgbBuffer) {
    encodeSC(rgbBuffer, 88.064, 0x38);
}

void encodeSCX(SSTV::rgb* rgbBuffer) {
    encodeSC(rgbBuffer, 345.600, 0x4C);
}