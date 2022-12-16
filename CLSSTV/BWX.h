#pragma once
#include "SSTV.h"
#include "wav.h"
#include "modes.h"

/*
    These are kind of a mess as BW8 and BW12 are not very well documented.
    All timing values were taken from the MMSSTV source code.
*/

void encodeBW(SSTV::rgb* rgbBuffer, double lineMS) {
    int img_width = BW8.size.X;
    int img_height = BW8.size.Y;

    float mspp = lineMS / static_cast<float>(img_width);
    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, 6.f);
        wav::addTone(1500, 2.f);
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp); //Luminance (Y) only, average of RGB also works.
        }
    }
}

void encodeBW8(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x02);
    encodeBW(rgbBuffer, 58.89709f);
}

void encodeBW12(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCodeManual(0x86);
    encodeBW(rgbBuffer, 92.f);
}