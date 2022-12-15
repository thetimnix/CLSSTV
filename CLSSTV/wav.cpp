#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <functiondiscoverykeys.h>
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

    void addTone(short frequency, float duration, generatorType gt) {
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
            switch (gt) {
                case GT_SINE:
                    wavData[writeIndex].S = (short)(ampl * sin(angle));
                    break;
                case GT_SQUARE:
                    wavData[writeIndex].S = (short)(ampl * (sin(angle) > 0 ? 1 : -1));
                    break;
                case GT_TRIANGLE:
                    wavData[writeIndex].S = (short)(ampl * (2 / pi) * asin(sin(angle)));
                    break;
            }
            angle += ((2 * pi * frequency) / header.sampleRate);
            writeIndex++;

            //balances issues with timing. see note at the bottom.
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

    void WASAPIListDevices() {
		IMMDeviceEnumerator* pEnumerator = NULL;
		IMMDeviceCollection* pCollection = NULL;
		UINT count;
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return;
        }
		hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to create device enumerator!\n");
			return;
		}
		hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to enumerate audio endpoints!\n");
			return;
		}
		hr = pCollection->GetCount(&count);
		if (FAILED(hr)) {
			printf_s("[ERR] Failed to get device count!\n");
			return;
		}

        printf_s("[PLAYBACK DEVICES]\n");
		for (UINT i = 0; i < count; i++) {
			IMMDevice* pDevice = NULL;
			IPropertyStore* pProps = NULL;
			PROPVARIANT varName;
			PropVariantInit(&varName);
			hr = pCollection->Item(i, &pDevice);
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to get device %d!\n", i);
				return;
			}
			hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to open property store for device %d!\n", i);
				return;
			}
			hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
			if (FAILED(hr)) {
				printf_s("[ERR] Failed to get friendly name for device %d!\n", i);
				return;
			}
            
            //list devices
			printf_s(" [%d]: %S\n", i, varName.pwszVal);
            
			PropVariantClear(&varName);
			pProps->Release();
			pDevice->Release();
		}
		pCollection->Release();
		pEnumerator->Release();
    }
    
    wchar_t* WASAPIGetDeviceIdByIndex(int index) {
        IMMDeviceEnumerator* pEnumerator = NULL;
        IMMDeviceCollection* pCollection = NULL;
        UINT count;
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return 0;
        }
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to create device enumerator!\n");
            return 0;
        }
        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to enumerate audio endpoints!\n");
            return 0;
        }
        hr = pCollection->GetCount(&count);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get device count!\n");
            return 0;
        }

        if (index < 0 || index > count) {
			printf_s("[ERR] Invalid device index, see -D\n");
			return 0;
        }
        
        wchar_t* deviceID = (wchar_t*)malloc(128);
        if (!deviceID) { return 0; }
        
        IMMDevice* pDevice = NULL;
        IPropertyStore* pProps = NULL;
        PROPVARIANT varName;
        PropVariantInit(&varName);
        hr = pCollection->Item(index, &pDevice);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get device!\n");
            return 0;
        }
        hr = pDevice->GetId((LPWSTR*)&deviceID);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to get ID of device!\n");
            return 0;
        }

        PropVariantClear(&varName);
        pDevice->Release();

        pCollection->Release();
        pEnumerator->Release();

        return deviceID;
    }

    void ShowConsoleCursor(bool showFlag)
    {
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(out, &cursorInfo);
        cursorInfo.bVisible = showFlag;
        SetConsoleCursorInfo(out, &cursorInfo);
    }
    
    void generateProgressBar(int progress, char* buffer, int bufferWidth) {
		int barWidth = bufferWidth - 2;
		int barProgress = (barWidth * progress) / 100;
		int barRemainder = barWidth - barProgress;
		for (int i = 0; i < barProgress; i++) { buffer[i] = '='; }
		for (int i = 0; i < barRemainder; i++) { buffer[i + barProgress] = '-'; }
    }

    //i have no fucking idea what this does, i copied it from an example and just monkey-typewriter'd it until it worked
    char progressBarTxt[50] = {};
    void beginPlayback(int iDeviceID) {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY);
        if (FAILED(hr)) {
            printf_s("[ERR] Failed to CoInitialize!\n");
            return;
        }
        
        IMMDeviceEnumerator* deviceEnumerator;
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (LPVOID*)(&deviceEnumerator));
        IMMDevice* audioDevice;
        hr = deviceEnumerator->GetDevice(WASAPIGetDeviceIdByIndex(iDeviceID), &audioDevice);
        if (FAILED(hr)) {
            return; //probably a failure in WASAPIGetDeviceIdByIndex, user was warned above, just quit
        }
        
        deviceEnumerator->Release();

        IAudioClient2* audioClient;
        hr = audioDevice->Activate(__uuidof(IAudioClient2), CLSCTX_ALL, nullptr, (LPVOID*)(&audioClient));

        audioDevice->Release();
        
        //pretty much the same as the wav header class
        WAVEFORMATEX mixFormat = {};
        mixFormat.wFormatTag = WAVE_FORMAT_PCM;
        mixFormat.nChannels = header.channels;
        mixFormat.nSamplesPerSec = header.sampleRate;
        mixFormat.wBitsPerSample = header.BPS;
        mixFormat.nBlockAlign = (mixFormat.nChannels * mixFormat.wBitsPerSample) / 8;
        mixFormat.nAvgBytesPerSec = mixFormat.nSamplesPerSec * mixFormat.nBlockAlign;

        //initialize the audio client
        DWORD initStreamFlags = (AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY);
        hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, initStreamFlags, (REFERENCE_TIME)10000000, 0, &mixFormat, nullptr);

        //initialize the renderer
        IAudioRenderClient* audioRenderClient;
        hr = audioClient->GetService(__uuidof(IAudioRenderClient), (LPVOID*)(&audioRenderClient));

        UINT32 bufferSizeInFrames;
        hr = audioClient->GetBufferSize(&bufferSizeInFrames);
        hr = audioClient->Start();

        int wavPlaybackSample = 0;
        int lastPrintedPercentage = 0;
        bool finished = false;
        
        ShowConsoleCursor(false);
        
        while (!finished)
        {
            //i dont know what the padding does
            UINT32 bufferPadding;
            hr = audioClient->GetCurrentPadding(&bufferPadding);

            //some form of maths ¯\_(._.)_/¯
            int soundBufferLatency = bufferSizeInFrames / 50;
            int numFramesToWrite = soundBufferLatency - bufferPadding;
            
            //gets the buffer from the audio client that you can proceed to squirt audio data into
            short* buffer;
            hr = audioRenderClient->GetBuffer(numFramesToWrite, (BYTE**)(&buffer));
            
            //data squirtery loop
            for (int frameIndex = 0; frameIndex < numFramesToWrite; ++frameIndex)
            {                    
				//squirt the audio data from the wav file into the playback buffer
                *buffer++ = ((short*)wavheap)[wavPlaybackSample];
                
                //progress bar code only
				int percentage = (int)((float)wavPlaybackSample / (float)writeIndex * 100.f);
				if (percentage > lastPrintedPercentage) {
					generateProgressBar(percentage, progressBarTxt, 50);
					printf_s("\r[PLAYING][%s][%i%%]", progressBarTxt, percentage); //i kinda feel this shouldnt be done like this. maybe play in another thread?
					lastPrintedPercentage = percentage;
				}
                
                //next sample
                ++wavPlaybackSample;
            }

            //if its done then quit out of the loop
            if (wavPlaybackSample > writeIndex) {
                finished = true;
                Sleep(1000);
                ShowConsoleCursor(true);
                break;
            }
            
            //free up the buffer because memory
            audioRenderClient->ReleaseBuffer(numFramesToWrite, 0);
        }

        audioClient->Stop();
        audioClient->Release();
        audioRenderClient->Release();
    }
    
    int save(FILE* fptr) {
        header.dataSize -= header.dataSize - bytesWritten;
        header.fileSize = header.dataSize + sizeof(wavHeader);
        memcpy(wavheap, &header, sizeof(header));
        return fwrite(wavheap, header.fileSize, 1, fptr);
    }
}

//todo: explain balancing again