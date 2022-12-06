#include "SSTV.h"

namespace tr {

	struct mapping {
		char c;
		int mapX;
		int mapY;
	};
	
	void initFont();
	extern SSTV::rgb* rgbFont;
}