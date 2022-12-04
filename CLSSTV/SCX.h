#pragma once
#include "SSTV.h"
#include "wav.h"

void encodeSC(SSTV::rgb* rgbBuffer, double lineMS) {
    int jpg_width = 320;
    int jpg_height = 256;

    float mspp = lineMS / (float)jpg_width;

    wav::addTone(1200, 9);
    for (int y = 0; y < jpg_height; y++) {
        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * jpg_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.g), mspp);
            (y * jpg_width) + x++;
        }

        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * jpg_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.b), mspp);
            (y * jpg_width) + x++;
        }

        wav::addTone(1200, 9);
        wav::addTone(1500, 1.5);
        for (int x = 0; x < jpg_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * jpg_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.r), mspp);
        }
    }
}

void encodeSC1(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x3C);
    encodeSC(rgbBuffer, 138.240);
}

void encodeSC2(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x38);
    encodeSC(rgbBuffer, 88.064);
}

void encodeSCDX(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x4C);
    encodeSC(rgbBuffer, 345.600);
}