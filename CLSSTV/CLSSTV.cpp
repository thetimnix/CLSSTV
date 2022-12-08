#include <iostream>
#include "SSTV.h"
#include "wav.h"
#include "jpgd.h"
#include "jpge.h"
#include "textRendering.h"
#include "modes.h"

#include "BWX.h" //BW8, BW12
#include "SCX.h" //Scottie1, Scottie2, ScottieDX
#include "R36.h" //Robot36
#include "R72.h" //Robot72
#include "PDX.h" //PD50, PD90, PD120
#include "MRX.h" //Martin1, Martin2 

const char* getFilenameFromPath(const char* path) {
	const char* filename = path;
	for (int x = strlen(path); x > 0; x--) {
		if (path[x] == '\/') {
			filename = &path[x + 1];
		}
	}
	return filename;
}

enum fileType {
	FT_ERR,
	FT_BMP,
	FT_JPG
};

fileType readFileType(const char* path) {
	//open file
	FILE* file;
	fopen_s(&file, path, "rb");
	if (!file) { return fileType::FT_ERR; }

	//alloc space and read the files header
	unsigned char header[2];
	fread(header, 1, 2, file);
	fclose(file);

	//compare file header and return
	if (header[0] == 'B' && header[1] == 'M') { return fileType::FT_BMP; }
	if (header[0] == 0xFF && header[1] == 0xD8) { return fileType::FT_JPG; }

	return fileType::FT_ERR;
}

SSTV::rgb* readBitmap(const char* path, int& width, int& height) {
	FILE* file;
	fopen_s(&file, path, "rb");
	if (!file) { return nullptr; }

	//read the header
	unsigned char header[54];
	fread(header, 1, 54, file);

	//get the width and height
	width = *(int*)&header[18];
	height = *(int*)&header[22];

	//get the bitdepth
	int bitDepth = *(int*)&header[28];

	//get the padding
	int padding = (4 - (width * (bitDepth / 8)) % 4) % 4;

	//allocate memory for the image
	SSTV::rgb* image = (SSTV::rgb*)malloc((width * height) * sizeof(SSTV::rgb));
	if (!image) { return nullptr; }

	//read the image
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			unsigned char pixel[3];
			fread(pixel, 1, 3, file);
			image[(height - y - 1) * width + x] = SSTV::rgb(pixel[2], pixel[1], pixel[0]);
		}
		fseek(file, padding, SEEK_CUR);
	}

	fclose(file);
	return image;
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


int main(int argc, char* argv[])
{	
	//output file pointer
	FILE* ofptr;

	//init drawing system
	tr::initFont();
	
	//print mode list if requested
	if (strcmp(argv[1], "-M") == 0) {
		printf_s(" %-8s | %-16s | %-9s\n", "CODE", "DESCRIPTION", "IMG SIZE");
		printf_s(" %-8s | %-16s | %-9s\n", "", "", "");
		for (encMode& line : modes) {
			printf_s(" %-8s | %-16s | %i x %i\n", line.code, line.desc, line.size.X, line.size.Y);
		}
		return 0;
	}

	//print usage if args are missing
	if (argc != 4) {
		printf_s("USAGE:\n Arg 1: Encode type (see -M)\n Arg 2: Input (JPG)\n Arg 3: Output (WAV)");
		return 0;
	}

	//begin encode
	printf_s("[CLSSTV R1.4 2022]\n");
	printf_s("[Beginning SSTV generation @ %iKHz]\n", wav::header.sampleRate);

	//read input jpg
	vec2 jpgSize = { 0, 0 };
	int idk = 4;
	SSTV::rgb* rgbBuffer = nullptr;

	switch (readFileType(argv[2])) {
		case FT_JPG:
			rgbBuffer = (SSTV::rgb*)jpgd::decompress_jpeg_image_from_file(argv[2], &jpgSize.X, &jpgSize.Y, &idk, 4);
			break;
			
		case FT_BMP:
			rgbBuffer = readBitmap(argv[2], jpgSize.X, jpgSize.Y);
			break;

		case FT_ERR:
			printf_s("[ERR] Could not read source file\n");
			return 0;
	}
	
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
	int openErrNo = fopen_s(&ofptr, argv[3], "wb");
	if (openErrNo != 0) {
		char errBuffer[256] = {};
		strerror_s(errBuffer, openErrNo);
		printf_s("[ERR] Could not open output file (%s)\n", errBuffer);
		return 0;
	}

	//find and set user selected encoding mode
	encMode* selectedEncMode = 0;
	SSTV::rgb* resizedRGB = 0;
	bool validEncMode = false;
	for (encMode& em : modes) {
		if (strcmp(argv[1], em.code) == 0) {
			//get enc mode
			selectedEncMode = &em;
			//resize image if required
			resizedRGB = resizeNN(rgbBuffer, jpgSize, em.size);
			//draw required text ontop of resizedRGB
			tr::drawString(resizedRGB, em.size, { 0, 0 }, "<3");
			//exit loop
			validEncMode = true;
			break;
		}
	}
	
	if(!validEncMode) {
		printf_s("[ERR] SSTV encode type not recognised, see -M\n");
		return 0;
	}
	
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
	
	printf_s("[Encode complete, wrote %i bytes to %s]\n", wav::header.fileSize, getFilenameFromPath(argv[3]));
	printf_s(" Expected time: %f MS\n", wav::expectedDurationMS);
	printf_s(" Actual time:   %f MS\n", wav::actualDurationMS);
	printf_s(" Added: %i Skipped: %i\n", wav::balance_AddedSamples, wav::balance_SkippedSamples);
}
