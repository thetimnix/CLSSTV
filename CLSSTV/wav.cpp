#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "wav.h"

namespace wav {
    int ampl = 30000;
    
    const float pi = 3.1415926535;
	double angle = 0.0;

	wavHeader header = {};
    void* wavheap = 0;

    double expectedDurationMS = 0;
    double actualDurationMS = 0;
    int balance_AddedSamples = 0;
    int balance_SkippedSamples = 0;
    int bytesWritten = 0;
    int writeIndex = 0;

    int init() {
        header.dataSize = (static_cast<unsigned long long>(header.sampleRate) * 15) * sizeof(short);
        header.fileSize = header.dataSize + sizeof(wavHeader);
        wavheap = malloc(header.fileSize);
        if (!wavheap) {
            return 0;
        }
        return 1;
    }

    void addTone(short frequency, float duration) {
        //number of samples required for the requested duration. sometimes.
        int sampleCount = round((header.sampleRate) * (duration / 1000.f));

        //balancing
        expectedDurationMS += duration;
        actualDurationMS += (sampleCount / static_cast<double>(48000)) * 1000;
        float msPerSample = 1000.f / header.sampleRate;

        //if you're gonna run out of space in the wav then add more in 15 second chunks.
        while (bytesWritten + (sampleCount * sizeof(short)) > header.dataSize) {
            header.dataSize += (header.sampleRate * 15) * sizeof(short);
            header.fileSize = header.dataSize + sizeof(wavHeader);
            void* reallocated = realloc(wavheap, header.fileSize);
            if (reallocated) { wavheap = reallocated; }
            else {
                printf_s("Issue while expanding WAV memory!\n");
                return;
            }
        }

        //actually calculate and add the data
        monoSample* wavData = (monoSample*)((uintptr_t)wavheap + sizeof(header));
        for (int i = 0; i < sampleCount; i++) {
            bytesWritten += (int)sizeof(short);

            //calculates the actual sine wave
            //these two lines took the longest in this entire project. fuck maths.
            wavData[writeIndex] = { (short)(ampl * sin(angle)) };
            angle += ((2 * pi * frequency) / header.sampleRate);
            writeIndex++;

            //balances issues with timing. see note.
            float diff = actualDurationMS - expectedDurationMS;
            if (diff > msPerSample) {
                sampleCount--;
                actualDurationMS -= msPerSample;
                balance_SkippedSamples++;
            }
            if (diff < -msPerSample) {
                sampleCount++;
                actualDurationMS += msPerSample;
                balance_AddedSamples++;
            }

            while (angle > 2 * pi) { angle -= 2 * pi; } //avoid floating point weirdness
        }
    }

    int save(FILE* fptr) {
        header.dataSize -= header.dataSize - bytesWritten;
        header.fileSize = header.dataSize + sizeof(wavHeader);
        memcpy(wavheap, &header, sizeof(header));
        return fwrite(wavheap, header.fileSize, 1, fptr);
    }
}