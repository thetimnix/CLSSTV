#pragma once

namespace wav {
    struct wavHeader {
        char riff[4] = { 'R', 'I', 'F', 'F' };
        int fileSize = 44;
        char wave[4] = { 'W', 'A', 'V', 'E' };
        char fmt[4] = { 'f', 'm', 't', 0x20 };
        int headerSize = 16;
        short format = 1;
        short channels = 1;
        int sampleRate = 48000;
        int SBC = 96000;
        short BC = 2;
        short BPS = 16;
        char data[4] = { 'd', 'a', 't', 'a' };
        int dataSize = 0;
    };

    struct monoSample {
        short S;
    };

    int init();
    void addTone(short frequency, float duration);
    int save(FILE* fptr);

    extern wavHeader header;
    extern double expectedDurationMS;
    extern double actualDurationMS;
    extern int balance_AddedSamples;
    extern int balance_SkippedSamples;
}


