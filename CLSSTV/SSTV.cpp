#include "SSTV.h"
#include "wav.h"

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

    //adds the first 7 bytes and uses 8 for parity
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

        if (ones % 2 == 0) {
            wav::addTone(1300, 30); //parity 0 if even
        }
        else {
            wav::addTone(1100, 30); //parity 1 if odd
        }

        wav::addTone(1200, 30);
    }

    //adds all 8 bytes of the input, without calculating the parity bit
    void addVisCodeManual(char visCode) {
        wav::addTone(1900, 300);
        wav::addTone(1200, 10);
        wav::addTone(1900, 300);
        wav::addTone(1200, 30);

        int bit = 0;
        for (int i = 0; i < 8; i++) {
            bit = (visCode >> i) & 1;
            if (bit) {
                wav::addTone(1100, 30); //1
            }
            else {
                wav::addTone(1300, 30); //0
            }
        }

        wav::addTone(1200, 30);
    }
}