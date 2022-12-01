#pragma once
#include "SSTV.h"
#include "wav.h"

void encodeBW8(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x02);

    int jpg_width = 160;
    int jpg_height = 120;

    float mspp = 60.f / static_cast<float>(jpg_width);
    int pxptr = 0;
    for (int y = 0; y < jpg_height; y++) {
        wav::addTone(1200, 7); //some sources say this should be 5, others say 10. 7 works.
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[pxptr];
            wav::addTone(1500 + (3.13 * ((c.r + c.g + c.b) / 3)), mspp);
            pxptr++;
        }
    }
}