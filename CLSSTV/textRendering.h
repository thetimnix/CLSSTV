#include "SSTV.h"

namespace tr {

	struct mapping {
		char c;
		int mapX;
		int mapY;
	};
	
	void initFont();
	void drawCharacter(SSTV::rgb* canvas, vec2 canvasSize, char c, vec2 pos);
	void drawString(SSTV::rgb* canvas, vec2 canvasSize, vec2 pos, const char* fmt...);
	extern SSTV::rgb* rgbFont;

}