#pragma once
#include "SSTV.h"
#include "wav.h"
#include "modes.h"

void encodeMR(SSTV::rgb* rgbBuffer, double lineMS) {
    int img_width = MR1.size.X;
    int img_height = MR1.size.Y;

    float mspp = lineMS / (float)img_width;
    float syncMs = 4.862f;
    float seprMs = 0.572f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, syncMs);
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.g), mspp);
        }
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.b), mspp);
        }
        wav::addTone(1500, seprMs);

        for (int x = 0; x < img_width; x++) {
            SSTV::rgb c = rgbBuffer[(y * img_width) + x];
            wav::addTone(1500 + (CFMultiplier * c.r), mspp);
        }
        wav::addTone(1500, seprMs);
    }
}

void encodeMR1(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x2C);
    encodeMR(rgbBuffer, 146.432f);
}

void encodeMR2(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x28);
    encodeMR(rgbBuffer, 73.216f);
}