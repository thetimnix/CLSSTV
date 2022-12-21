#pragma once
#include "SSTV.h"

enum encModeID {
	EM_BW8,
	EM_BW12,
	EM_R24,
	EM_R36,
	EM_R72,
	EM_SC1,
	EM_SC2,
	EM_SCDX,
	EM_MR1,
	EM_MR2,
	EM_PD50,
	EM_PD90,
	EM_PD120
};

struct encMode {
	encModeID ID;
	char code[8];
	char desc[128];
	vec2 size;
};

extern encMode BW8;
extern encMode BW12;
extern encMode R24;
extern encMode R36;
extern encMode R72;
extern encMode SC1;
extern encMode SC2;
extern encMode SCDX;
extern encMode MR1;
extern encMode MR2;
extern encMode PD50;
extern encMode PD90;
extern encMode PD120;
extern encMode modes[13];