#include "modes.h"

encMode BW8 =   { EM_BW8,   "BW8",   "Black/White 8s",  {160, 120} };
encMode BW12 =  { EM_BW12,  "BW12",  "Black/White 12s", {160, 120} };
encMode R36 =   { EM_R36,   "R36",   "Robot36",         {320, 240} };
encMode R72 =   { EM_R72,   "R72",   "Robot72",         {320, 240} };
encMode SC1 =   { EM_SC1,   "SC1",   "Scottie 1",       {320, 256} };
encMode SC2 =   { EM_SC2,   "SC2",   "Scottie 2",       {320, 256} };
encMode SCDX =  { EM_SCDX,  "SCDX",  "Scottie DX",      {320, 256} };
encMode MR1 =   { EM_MR1,   "MR1",   "Martin 1",        {320, 256} };
encMode MR2 =   { EM_MR2,   "MR2",   "Martin 2",        {320, 256} };
encMode PD50 =  { EM_PD50,  "PD50",  "PD50",            {320, 256} };
encMode PD90 =  { EM_PD90,  "PD90",  "PD90",            {320, 256} };
encMode PD120 = { EM_PD120, "PD120", "PD120",           {640, 496} };
encMode modes[] = { BW8, BW12, R36, R72, SC1, SC2, SCDX, MR1, MR2, PD50, PD90, PD120 };