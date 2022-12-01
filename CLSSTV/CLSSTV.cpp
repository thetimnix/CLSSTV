#include <iostream>
#include "SSTV.h"
#include "wav.h"
#include "jpgd.h"

#include "BW8.h"
#include "SC1.h"
#include "R36.h"

const char* getFilenameFromPath(const char* path) {
	int slashIndex = -1;
	int postSlashStringLength = -1;
	for (int x = strlen(path); x > 0; x--) {
		if (path[x] == '\/') {
			slashIndex = x;
			postSlashStringLength = strlen(path) - x;
		}
	}
	if (slashIndex > -1) {
		char* processedFileName = (char*)malloc(postSlashStringLength);
		if (!processedFileName) { return path; }
		sprintf_s(processedFileName, postSlashStringLength, path + (slashIndex + 1));
		return processedFileName;
	}
	return path;
}

const char* encodeTypes[] = {
	"R36 : Robot36 (320 x 240)",
	"SC1 : Scottie1 (320 x 256)",
	"BW8 : Black/White 8s (160 x 120)"
};

int main(int argc, char* argv[])
{
	//output file pointer
	FILE* ofptr;

	//print usage if args are missing
	if (argc != 4) {
		printf_s("USAGE:\n Arg 1: Encode type (see -modes)\n Arg 2: Input (JPG)\n Arg 3: Output (WAV)");
		return 0;
	}

	//print mode list
	if (strcmp(argv[1], "-modes") == 0) {
		for (const char* enType : encodeTypes) {
			printf_s(" %s\n", enType);
		}
		return 0;
	}

	//begin encode
	printf_s("[Beginning SSTV generation @ %iKHz]\n", wav::header.sampleRate);

	//read input jpg
	int jpg_width = 0;
	int jpg_height = 0;
	int idk = 4;
	SSTV::rgb* rgbBuffer = (SSTV::rgb*)jpgd::decompress_jpeg_image_from_file(argv[2], &jpg_width, &jpg_height, &idk, 4);
	if (!rgbBuffer) { 
		printf_s("Failed to open .jpg\n"); 
		return 0; 
	}

	//init wav system
	if (!wav::init()) {
		printf_s("Issue while allocating WAV memory!\n");
	}
	
	//open output file
	fopen_s(&ofptr, argv[3], "wb");
	if (!ofptr) {
		printf_s("Issue opening output file!\n");
		return 0;
	}

	//call encodes
	if (strcmp(argv[1], "BW8") == 0) {
		if (!(jpg_width == 160 && jpg_height == 120)) {
			printf_s("Incorrectly sized image supplied. Required 160 x 120.\n");
			return 0;
		}
		encodeBW8(rgbBuffer);
	}
	if (strcmp(argv[1], "SC1") == 0) {
		if (!(jpg_width == 320 && jpg_height == 256)) {
			printf_s("Incorrectly sized image supplied. Required 320 x 256.\n");
			return 0;
		}
		encodeSC1(rgbBuffer);
	}
	if (strcmp(argv[1], "R36") == 0) {
		if (!(jpg_width == 320 && jpg_height == 240)) {
			printf_s("Incorrectly sized image supplied. Required 320 x 240.\n");
			return 0;
		}
		encodeR36(rgbBuffer);
	}

	//save and exit
	wav::save(ofptr);

	printf_s("[Encode complete, wrote %i bytes to %s]\n", wav::header.fileSize, getFilenameFromPath(argv[3]));
	printf_s(" Expected time: %f MS\n", wav::expectedDurationMS);
	printf_s(" Actual time:   %f MS\n", wav::actualDurationMS);
	printf_s(" Added: %i Skipped: %i\n", wav::balance_AddedSamples, wav::balance_SkippedSamples);
}
