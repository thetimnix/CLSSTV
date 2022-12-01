#include "SSTV.h"
#include "wav.h"
#include "jpgd.h"

namespace SSTV {
	int clampUC(int input) {
		return (input) > 255 ? 255 : (input) < 0 ? 0 : input;
	}

    void addVoxTone() {
        wav::addTone(1900, 100);
        wav::addTone(1500, 100);
        wav::addTone(1900, 100);
        wav::addTone(1500, 100);
        wav::addTone(2300, 100);
        wav::addTone(1500, 100);
        wav::addTone(2300, 100);
        wav::addTone(1500, 100);
    }

    void addVisCode(char visCode) {
        wav::addTone(1900, 300);
        wav::addTone(1200, 10);
        wav::addTone(1900, 300);
        wav::addTone(1200, 30);

        int bit = 0;
        int ones = 0;

        for (int i = 0; i < 7; i++) {
            bit = (visCode >> i) & 1;
            if (bit) {
                ones++;
                wav::addTone(1100, 30); //1
            }
            else {
                wav::addTone(1300, 30); //0
            }
        }
        if (ones % 2 == 1) { wav::addTone(1100, 30); } //parity 1
        else { wav::addTone(1300, 30); } //parity 0
        wav::addTone(1200, 30);
    }
}