#ifndef PROLOGIX_PARSER_H
#define PROLOGIX_PARSER_H

#include "Arduino.h"
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "AR488_Modules.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"



/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                       PROLOGIX PARSER                       ||
||                                                             ||
\*=============================================================*/



// Prologix controller functions
#ifdef USE_PROLOGIX_COMMON
  #include "Prologix_Common.h"
#endif

// Prologix controller functions
#ifdef USE_PROLOGIX_CONTROLLER
  #include "Prologix_Controller.h"
#endif

// Prologix devicve functions
#ifdef USE_PROLOGIX_DEVICE
  #include "Prologix_Device.h"
#endif

// ATTN handler
#ifdef USE_RATTN_HANDLER
  #include "RATTN_Handler.h"
#endif

// EEPROM handler
#ifdef USE_EEPROM_HANDLER
  #include "EEPROM_Handler.h"
#endif

// Macro helper
#ifdef USE_MACRO_HELPER
  #include "Macro_Helper.h"
#endif

// Diagnistic helper
#ifdef USE_DIAGNOSTIC_HELPER
  #include "Diagnostic_Helper.h"
#endif



/************************/
/***** PARSER CLASS *****/
/******vvvvvvvvvvvv******/

class prologixParser {

  public:

    prologixParser(inputBuffer& inBuf, GPIBbus& bus, utilityHelper& utils, AR488modulesList& modules);

    void init();
    void parse();
//    void parseMacro(char *& macro, size_t msize);
    void parseMacro(macroHandlerPM& mhandler);
    void reset();
    void parseInput(char c);
    uint8_t ready();
    size_t count();
    char * data();
    void setReadOnly(bool isro);

    #ifdef USE_RATTN_HANDLER
      void setTalkNow(bool talk);
    #endif

  private:

    inputBuffer& _inBuf;
    GPIBbus& _gpibBus;
    utilityHelper& _utils;
    AR488modulesList& _modules;

    void findExecCmd(char * cmd, uint8_t clen);
    void printHelp(char * token, bool find);
    void checkState();

    uint8_t _state;
    bool _isEsc;
    bool _isPlusEsc;
    bool _idnQuery;

    #ifdef USE_RATTN_HANDLER
      bool _talk;
    #endif
};

/******^^^^^^^^^^^^******/
/***** PARSER CLASS *****/
/************************/



#endif    // PROLOGIX_PARSER
