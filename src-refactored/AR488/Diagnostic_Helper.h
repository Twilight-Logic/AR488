#ifndef DIAGNOSTIC_HELPER_H
#define DIAGNOSTIC_HELPER_H

#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                      DIAGNOSTIC HELPER                      ||
||                                                             ||
\*=============================================================*/


#ifdef USE_DIAGNOSTIC_HELPER


// #define DIAG_FUNC_TYPE_VOID  0x40
#define DIAG_FUNC_TYPE_PARAM 0x80

extern bool isVerb;


class Diagnostix {

  public:

    Diagnostix(GPIBbus& bus, utilityHelper& utils);

    int execCmd(char * cmd, char * params);
    int getHelp(char * token, bool find);

    void printCurrentPinState();
    void testCtrlBusValue(uint8_t value);
    void testDataBusValue(uint8_t value);
    void asciiOut();
    void hexOut();
    void toggleTE();
    void toggleDC();
    void toggleSC();

  private:

    // GPIB bus object
    GPIBbus& gpibBus;
    utilityHelper& _util;

    /***** Comand names and descriptions struct *****/    
    struct dgnCmdTxtIdx {
      const char* token;
      const uint8_t idx;
      const char* desc;
    };

    /***** Comand function record - parameter functions *****/
    struct dgnCmdRecChar { 
      void (Diagnostix::*handler)(char *);
    };

    // Struct instance declarations
    static const dgnCmdTxtIdx dgnCmdIndex[];
    static const dgnCmdRecChar dgnCmdHidxChar[];

    bool _byteRead = false;
    bool _atnAsserted = false;

    int runCmd(char * cmd, char * params);
    int getHelpInfo(char * token, bool find);
//    void printHelpInfo(const __FlashStringHelper * token, const __FlashStringHelper * desc);

    void printDbPinout();
    void printCtrlPinout();
    void printPin(const __FlashStringHelper* pinid, uint8_t pin);

    void xdiag_h(char *params);

};

#endif  // USE_DIAGNOSTIC_HELPER

#endif  // DIAGNOSTIC_HELPER_H

