#pragma once
#include "SSTV.h"
#include "wav.h"
#include "modes.h"

//R24 feels like a missed opportunity, it could have been much faster using the R36 style colouring of sending Y and U on alternating lines

void encodeR24(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x04);

    int img_width = R24.size.X;
    int img_height = R24.size.Y;

    float mspp_Y = 92.f / (float)img_width;
    float mspp_UV = 46.f / (float)img_width;

    float hSyncMs = 6.f;
    float syncPorchMs = 2.f;
    float separatorMs = 3.f;
    float sepPorchMs = 1.f;

    for (int y = 0; y < img_height; y++) {
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp_Y);
        }

        wav::addTone(1500, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //V scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.v), mspp_UV);
        }

        wav::addTone(2300, separatorMs);
        wav::addTone(1900, sepPorchMs);

        //U scan
        for (int x = 0; x < img_width; x++) {
            SSTV::yuv c(rgbBuffer[(y * img_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.u), mspp_UV);
        }
    }
}