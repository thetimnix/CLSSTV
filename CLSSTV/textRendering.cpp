#include "textRendering.h"

namespace tr {
	SSTV::rgb* rgbFont = 0;
	
	SSTV::rgb* decompressFontData(char* compressed, int size) {
		SSTV::rgb* decompressed = new SSTV::rgb[size * 8];

		for (int p = 0; p < size; p++) {
			char* current = &compressed[p];

			for (int i = 0; i < 8; i++) {
				if (*current & (1 << i)) {
					decompressed[p * 8 + i] = SSTV::rgb(0xFF, 0xFF, 0xFF);
				}
				else {
					decompressed[p * 8 + i] = SSTV::rgb(0x00, 0x00, 0x00);
				}
			}
		}
		return decompressed;
	}

	void drawHeaderString(const char* header) {
		
	}
	
	void initFont() {
		rgbFont = decompressFontData((char*)tr::compressedFontData, sizeof(tr::compressedFontData));
	}
}

