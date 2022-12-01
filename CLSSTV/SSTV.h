#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

namespace SSTV {
    int clampUC(int input);

    struct rgb {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;

        rgb(unsigned char R, unsigned char G, unsigned char B) {
            r = R;
            g = G;
            b = B;
            a = 0x00;
        }
    };

    struct yuv {
        unsigned char y;
        unsigned char u;
        unsigned char v;

        yuv(rgb rgb) {
            y = clampUC(((66 * (rgb.r) + 129 * (rgb.g) + 25 * (rgb.b) + 128) >> 8) + 16);
            u = clampUC(((-38 * (rgb.r) - 74 * (rgb.g) + 112 * (rgb.b) + 128) >> 8) + 128);
            v = clampUC(((112 * (rgb.r) - 94 * (rgb.g) - 18 * (rgb.b) + 128) >> 8) + 128);
        }
    };

    void addVoxTone();
    void addVisCode(char visCode);
}

