#include <iostream>
#include <Windows.h>
#include <commdlg.h>

#include "SSTV.h"
#include "wav.h"
#include "textRendering.h"
#include "modes.h"

#include "BWX.h" //BW8, BW12
#include "SCX.h" //Scottie1, Scottie2, ScottieDX
#include "R36.h" //Robot36
#include "R72.h" //Robot72
#include "PDX.h" //PD50, PD90, PD120
#include "MRX.h" //Martin1, Martin2 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const char* getFilenameFromPath(const char* path) {
	const char* filename = path;
	for (int x = strlen(path); x > 0; x--) {
		if (path[x] == '\/') {
			filename = &path[x + 1];
		}
	}
	return filename;
}

SSTV::rgb* resizeNN(SSTV::rgb* input, vec2 inputSize, vec2 newSize) {
	//dont need to do anything if its already the right size, return the origional to save memory
	if (inputSize == newSize) {
		return input;
	}
	
	printf_s("[Resizing. This is not recommended, resize manually first!]\n");
	
	SSTV::rgb* output = new SSTV::rgb[newSize.Y * newSize.X];
	if (!output) { return nullptr; }

	//calc scale values
	float xScale = (float)newSize.X / (float)inputSize.X;
	float yScale = (float)newSize.Y / (float)inputSize.Y;

	for (int y = 0; y < newSize.Y; y++) {
		for (int x = 0; x < newSize.X; x++) {
			//get the nearest pixel in the input image using the x and y scale values
			int writeIndex = y * newSize.X + x;
			int readIndex = (int)(y / yScale) * inputSize.X + (int)(x / xScale);

			//set the pixel to the closest value, avoid any over/underflows. VS still complains about the possibility.
			if (writeIndex < (newSize.Y * newSize.X) && readIndex < (inputSize.X * inputSize.Y) && writeIndex >= 0 && readIndex >= 0) {
				output[writeIndex] = input[readIndex];
			}
		}
	}

	return output;
}

void printHelp() {
	printf_s("[CLSSTV USAGE]\n");
	printf_s("CLSSTV.exe -M [MODE] -I [INPUT.XXX] -O [OUTPUT.WAV]\n");
	printf_s("\nRequired:\n");
	printf_s(" -M: Encode mode\n");
	printf_s(" -I: Input path  (JPG / PNG / BMP / GIF)\n");
	printf_s(" -O: Output path (WAV)\n");
	printf_s("\nOptional / util:\n");
	printf_s(" -P: Play generated audio, use with or instead of -O\n");
	printf_s(" -L: List encode modes\n");
	printf_s(" -H: Show this help text\n");
}

void printEncCodes() {
	printf_s("[CLSSTV MODES]");
	printf_s(" %-8s | %-16s | %-9s\n", "CODE", "DESCRIPTION", "IMG SIZE");
	printf_s(" %-8s | %-16s | %-9s\n", "", "", "");
	for (encMode& line : modes) {
		printf_s(" %-8s | %-16s | %i x %i\n", line.code, line.desc, line.size.X, line.size.Y);
	}
}

int main(int argc, char* argv[])
{	
	//print help texts if required
	if (strcmp(argv[1], "-L") == 0) {
		printEncCodes();
		return 0;
	}
	
	if (strcmp(argv[1], "-H") == 0) {
		printHelp();
		return 0;
	}
	
	//output file pointer
	FILE* ofptr;

	//init drawing system
	tr::initFont();

	//process other arguments
	encMode* selectedEncMode = 0;
	bool validEncMode = false;
	
	char* inputPath = 0;
	char* outputPath = 0;
	bool playback = false;
	
	for (int i = 0; i < argc; i++) {
		//find input argument
		if (strcmp(argv[i], "-I") == 0 && i + 1 <= argc) {
			inputPath = argv[i + 1];
		}
		
		//find output argument
		if (strcmp(argv[i], "-O") == 0 && i + 1 <= argc) {
			outputPath = argv[i + 1];
		}

		//find encode mode
		if (strcmp(argv[i], "-M") == 0 && i + 1 <= argc && !validEncMode) {
			for (encMode& em : modes) {
				if (strcmp(argv[i + 1], em.code) == 0) {
					selectedEncMode = &em;
					validEncMode = true;
				}
			}
		}
		
		//find -P switch
		if (strcmp(argv[i], "-P") == 0) {
			playback = true;
		}
	}

	//validation stuff
	if (!inputPath || !(playback || outputPath) || !validEncMode) {
		printf_s("%i, %i, %i, %i\n", (bool)inputPath, playback, (bool)outputPath, validEncMode);
		printHelp();
		return 0;
	}

	if (playback) {
		printf_s("[ERR] -P is not implemented. Quitting.\n");
		return 0;
	}
	
	//begin encode
	printf_s("[CLSSTV R1.4 2022]\n");
	printf_s("[Beginning SSTV generation @ %iKHz]\n", wav::header.sampleRate);

	//read input jpg
	vec2 imgSize = { 0, 0 };
	int imgChannels = 4;
	SSTV::rgb* rgbBuffer = nullptr;

	//stbi will load most image types, dont need to determin which load function to use anymore
	rgbBuffer = (SSTV::rgb*)stbi_load(inputPath, &imgSize.X, &imgSize.Y, &imgChannels, 3);
	
	if (!rgbBuffer) { 
		printf_s("[ERR] Could not read source file\n");
		return 0;
	}

	//init wav system
	if (!wav::init()) {
		printf_s("[ERR] Could not allocate WAV memory\n");
		return 0;
	}
	
	//open output file
	int openErrNo = fopen_s(&ofptr, outputPath, "wb");
	if (openErrNo != 0) {
		char errBuffer[256] = {};
		strerror_s(errBuffer, openErrNo);
		printf_s("[ERR] Could not open output file (%s)\n", errBuffer);
		return 0;
	}

	//resize if required
	SSTV::rgb* resizedRGB = 0;
	resizedRGB = resizeNN(rgbBuffer, imgSize, selectedEncMode->size);

	//draw overlay
	tr::drawString(resizedRGB, selectedEncMode->size, { 0, 0 }, "CLSSTV");
	
	//add VOX tone
	SSTV::addVoxTone();

	//call actual encode function
	switch (selectedEncMode->ID) {
		case(EM_BW8):
			encodeBW8(resizedRGB);
			break;
		
		case(EM_BW12):
			encodeBW12(resizedRGB);
			break;
		
		case(EM_R36):
			encodeR36(resizedRGB);
			break;
		
		case(EM_R72):
			encodeR72(resizedRGB);
			break;
		
		case(EM_SC1):
			encodeSC1(resizedRGB);
			break;

		case(EM_SC2):
			encodeSC2(resizedRGB);
			break;

		case(EM_SCDX):
			encodeSCDX(resizedRGB);
			break;

		case(EM_MR1):
			encodeMR1(resizedRGB);
			break;

		case(EM_MR2):
			encodeMR2(resizedRGB);
			break;

		case(EM_PD50):
			encodePD50(resizedRGB);
			break;

		case(EM_PD90):
			encodePD90(resizedRGB);
			break;

		case(EM_PD120):
			encodePD120(resizedRGB);
			break;
		
		default:
			printf_s("[ERR] Invalid encCode ID");
			return 0;
	}
	
	//save and exit
	if (wav::save(ofptr) <= 0) {
		char errBuffer[256] = {};
		strerror_s(errBuffer, errno);
		printf_s("[ERR] Issue opening output file (%s)\n", errBuffer);
		return 0;
	}
	
	printf_s("[Encode complete, wrote %i bytes to %s]\n", wav::header.fileSize, getFilenameFromPath(outputPath));
	printf_s(" Expected time: %f MS\n", wav::expectedDurationMS);
	printf_s(" Actual time:   %f MS\n", wav::actualDurationMS);
	printf_s(" Added: %i Skipped: %i\n", wav::balance_AddedSamples, wav::balance_SkippedSamples);
}
