#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>
#include "AR488_ComPorts.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                       UTILITY HELPER                        ||
||                                                             ||
\*=============================================================*/


extern bool isVerb;

#define GPIB_ASSERTED   true
#define GPIB_UNASSERTED false


class utilityHelper {

  public:

    utilityHelper();
    void mcuReset();
    bool isNumber(char * numstr);
    bool isRange(char * rangestr, size_t rsize, unsigned long values[2]);
    bool notInRange(char *param, uint16_t lowl, uint16_t higl, uint16_t &rval);
    void errorMsg(int err);
    void showPrompt();
    void printHelpLine(const __FlashStringHelper * token, const __FlashStringHelper * desc);
//    bool waitForPinState(uint8_t pin, uint8_t state, unsigned long tmo);
    bool waitForPinState(uint8_t pin, bool asserted, unsigned long tmo);

  private:


};


#endif    // UTILITIES
