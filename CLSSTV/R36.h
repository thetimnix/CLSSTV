#pragma once
#include "SSTV.h"
#include "wav.h"

void encodeR36(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x08);

    int jpg_width = 320;
    int jpg_height = 240;

    float mspp_Y = 88.f / (float)jpg_width;
    float mspp_UV = 44.f / (float)jpg_width;

    float hSyncMs = 9.f;
    float syncPorchMs = 3.f;
    float separatorMs = 4.5f;
    float sepPorchMs = 1.5f;

    int pxptr = 0;
    for (int y = 0; y < jpg_height; y++) {

        int tmpPxptr = pxptr;
        wav::addTone(1200, hSyncMs);
        wav::addTone(1500, syncPorchMs);

        //Y scan
        for (int x = 0; x < jpg_width; x++) {
            SSTV::yuv c(rgbBuffer[pxptr]);
            wav::addTone(1500 + (3.13 * c.y), mspp_Y);
            pxptr++;
        }
        pxptr = tmpPxptr;

        if (y % 2 == 0) {
            //evens
            wav::addTone(1500, separatorMs);
            wav::addTone(1900, sepPorchMs);

            //V scan
            for (int x = 0; x < jpg_width; x++) {
                SSTV::yuv c(rgbBuffer[pxptr]);
                wav::addTone(1500 + (3.13 * c.v), mspp_UV);
                pxptr++;
            }
        }
        else {
            //odds
            wav::addTone(2300, separatorMs);
            wav::addTone(1900, sepPorchMs);

            //U scan
            for (int x = 0; x < jpg_width; x++) {
                SSTV::yuv c(rgbBuffer[pxptr]);
                wav::addTone(1500 + (3.13 * c.u), mspp_UV);
                pxptr++;
            }
        }
    }
}