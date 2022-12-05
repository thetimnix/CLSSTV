#pragma once
#include "SSTV.h"
#include "wav.h"

void encodeR72(SSTV::rgb * rgbBuffer) {
    SSTV::addVisCode(0x0C);

    int jpg_width = 320;
    int jpg_height = 240;

    float mspp_Y = 138.f / (float)jpg_width;
    float mspp_UV = 69.f / (float)jpg_width;

    float hSyncMs = 9.f;
    float syncPorchMs = 3.f;
    float separatorMs = 4.5f;
    float sepPorchMs = 1.5f;

    for (int y = 0; y < jpg_height; y++) {
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < jpg_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp_Y);
        }

        wav::addTone(1500, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //V scan
        for (int x = 0; x < jpg_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.v), mspp_UV);
        }

        wav::addTone(2300, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //U scan
        for (int x = 0; x < jpg_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.u), mspp_UV);
        }
    }
}