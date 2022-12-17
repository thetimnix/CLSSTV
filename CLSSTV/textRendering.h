#include "SSTV.h"

namespace tr {

	struct mapping {
		char c;
		int mapX;
		int mapY;
	};
	
	extern SSTV::rgb white;
	extern SSTV::rgb black;
	extern SSTV::rgb red;
	extern SSTV::rgb green;
	extern SSTV::rgb blue;
	extern SSTV::rgb yellow;
	extern SSTV::rgb cyna;
	extern SSTV::rgb violet;
	
	void initFont();
	void drawString(SSTV::rgb colour, int fontSize, const char* fmt...);
	void setTextOrigin(vec2 origin);
	void bindToCanvas(SSTV::rgb* canvas, vec2 canvasSize);
	extern SSTV::rgb* rgbFont;

}