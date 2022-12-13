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
	void drawLine(SSTV::rgb* canvas, vec2 canvasSize, vec2 pos1, vec2 pos2);
	extern SSTV::rgb* rgbFont;

}