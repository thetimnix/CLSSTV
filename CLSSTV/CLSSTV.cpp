#include <iostream>
#include "SSTV.h"
#include "wav.h"
#include "jpgd.h"
#include "jpge.h"
#include "textRendering.h"

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
	FILE* file;
	fopen_s(&file, path, "rb");
	if (!file) { return fileType::FT_ERR; }

	unsigned char header[2];
	fread(header, 1, 2, file);
	fclose(file);

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

struct vec2 {
	int X;
	int Y;

	bool operator == (const vec2& rhs)
	{
		if (X == rhs.X && Y == rhs.Y) {
			return true;
		}
		return false;
	}

	bool operator != (const vec2& rhs)
	{
		return !(*this == rhs);
	}
};

void sizeErr(vec2 size) {
	printf_s("[ERR] Incorrectly sized image supplied. Required %i x %i.\n", size.X, size.Y);
}

struct encMode {
	char code[8];
	char desc[128];
	vec2 size;
};

encMode BW8 =   { "BW8",   "Black/White 8s",  {160, 120} };
encMode BW12 =  { "BW12",  "Black/White 12s", {160, 120} };
encMode R36 =   { "R36",   "Robot36",         {320, 240} };
encMode R72 =   { "R72",   "Robot72",         {320, 240} };
encMode SC1 =   { "SC1",   "Scottie1",        {320, 256} };
encMode SC2 =   { "SC2",   "Scottie2",        {320, 256} };
encMode SCDX =  { "SCDX",  "ScottieDX",       {320, 256} };
encMode MR1 =   { "MR1",   "Martin1",         {320, 256} };
encMode MR2 =   { "MR2",   "Martin2",         {320, 256} };
encMode PD50 =  { "PD50",  "PD50",            {320, 256} };
encMode PD90 =  { "PD90",  "PD90",            {320, 256} };
encMode PD120 = { "PD120", "PD120",           {640, 496} };

encMode modes[] = { BW8, BW12, R36, R72, SC1, SC2, SCDX, MR1, MR2, PD50, PD90, PD120 };

struct RGBPure {
	unsigned char R;
	unsigned char G;
	unsigned char B;
};

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

	//call individual encoders
	if (strcmp(argv[1], BW8.code) == 0) {
		if (jpgSize != BW8.size) {
			sizeErr(BW8.size);
			return 0;
		}
		encodeBW8(rgbBuffer);
	}
	else if (strcmp(argv[1], BW12.code) == 0) {
		if (jpgSize != BW12.size) {
			sizeErr(BW12.size);
			return 0;
		}
		encodeBW12(rgbBuffer);
	}
	else if (strcmp(argv[1], SC1.code) == 0) {
		if (jpgSize != SC1.size) {
			sizeErr(SC1.size);
			return 0;
		}
		encodeSC1(rgbBuffer);
	}
	else if (strcmp(argv[1], SC2.code) == 0) {
		if (jpgSize != SC2.size) {
			sizeErr(SC2.size);
			return 0;
		}
		encodeSC2(rgbBuffer);
	}
	else if (strcmp(argv[1], SCDX.code) == 0) {
		if (jpgSize != SCDX.size) {
			sizeErr(SCDX.size);
			return 0;
		}
		encodeSCDX(rgbBuffer);
	}
	else if (strcmp(argv[1], MR1.code) == 0) {
		if (jpgSize != MR1.size) {
			sizeErr(MR1.size);
			return 0;
		}
		encodeMR1(rgbBuffer);
	}
	else if (strcmp(argv[1], MR2.code) == 0) {
		if (jpgSize != MR2.size) {
			sizeErr(MR2.size);
			return 0;
		}
		encodeMR2(rgbBuffer);
	}
	else if (strcmp(argv[1], R36.code) == 0) {
		if (jpgSize != R36.size) {
			sizeErr(R36.size);
			return 0;
		}
		encodeR36(rgbBuffer);
	}
	else if (strcmp(argv[1], R72.code) == 0) {
		if (jpgSize != R72.size) {
			sizeErr(R72.size);
			return 0;
		}
		encodeR72(rgbBuffer);
	}
	else if (strcmp(argv[1], PD50.code) == 0) {
		if (jpgSize != PD50.size) {
			sizeErr(PD50.size);
			return 0;
		}
		encodePD50(rgbBuffer);
	}
	else if (strcmp(argv[1], PD90.code) == 0) {
		if (jpgSize != PD90.size) {
			sizeErr(PD90.size);
			return 0;
		}
		encodePD90(rgbBuffer);
	}
	else if (strcmp(argv[1], PD120.code) == 0) {
		if (jpgSize != PD120.size) {
			sizeErr(PD120.size);
			return 0;
		}
		encodePD120(rgbBuffer);
	}
	else {
		printf_s("[ERR] SSTV encode type not recognised, see -M\n");
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
