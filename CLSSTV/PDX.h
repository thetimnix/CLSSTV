#pragma once
#include "SSTV.h"
#include "wav.h"

//Timings arent quite right but MMSSTV and RX-SSTV can lock on fine, and its done to specifications
//(https://www.yumpu.com/en/document/read/7259948/proposal-for-sstv-mode-specifications)

void encodePD(SSTV::rgb* rgbBuffer, int jpg_width, int jpg_height, double lineMS) {
    float mspp = lineMS / (float)jpg_width;

    float syncMs = 20.f;
    float porchMs = 2.08f;

    for (int y = 0; y < jpg_height; y += 2) {
        wav::addTone(1200, syncMs);
        wav::addTone(1500, porchMs);

        //Y of even line
        for (int x = 0; x < jpg_width; x++) { //Y
            SSTV::yuv c(rgbBuffer[(y * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp);
        }

        //Avg V and U of both lines
        for (int x = 0; x < jpg_width; x++) { //V
            SSTV::yuv c1(rgbBuffer[(y * jpg_width) + x]);
            SSTV::yuv c2(rgbBuffer[((y + 1) * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * ((c1.v + c2.v) / 2)), mspp);
        }

        for (int x = 0; x < jpg_width; x++) { //U
            SSTV::yuv c1(rgbBuffer[(y * jpg_width) + x]);
            SSTV::yuv c2(rgbBuffer[((y + 1) * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * ((c1.u + c2.u) / 2)), mspp);
        }

        //Y of odd line
        for (int x = 0; x < jpg_width; x++) { //Y
            SSTV::yuv c(rgbBuffer[((y + 1) * jpg_width) + x]);
            wav::addTone(1500 + (CFMultiplier * c.y), mspp);
        }
    }
}

void encodePD50(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x5D);
    encodePD(rgbBuffer, 320, 240, 91.52f);
}

void encodePD90(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x63);
    encodePD(rgbBuffer, 320, 240, 170.240f);
}

void encodePD120(SSTV::rgb* rgbBuffer) {
    SSTV::addVisCode(0x5F);
    encodePD(rgbBuffer, 640, 496, 121.6f);
}