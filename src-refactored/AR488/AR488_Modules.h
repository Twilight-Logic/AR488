#ifndef AR488_MODULES_H
#define AR488_MODULES_H


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                      MODULE DEFINITIONS                     ||
||                                                             ||
\*=============================================================*/


#ifdef USE_PROLOGIX_COMMON 
  #include "Prologix_Common.h"
#endif
#ifdef USE_PROLOGIX_CONTROLLER
  #include "Prologix_Controller.h"
#endif
#ifdef USE_PROLOGIX_DEVICE
  #include "Prologix_Device.h"
#endif
#ifdef USE_RATTN_HANDLER
  #include "RATTN_Handler.h"
#endif
#ifdef USE_EEPROM_HANDLER
  #include "EEPROM_Handler.h"
#endif
#ifdef USE_MACRO_HANDLER_PM
  #include "Macro_Handler_PM.h"
#endif
#ifdef USE_DIAGNOSTIC_HELPER
  #include "Diagnostic_Helper.h"
#endif


struct AR488modulesList {
  #ifdef USE_PROLOGIX_COMMON
    prologixCommon * pcommon = NULL;
  #endif
  #ifdef USE_PROLOGIX_CONTROLLER
    prologixController * pcontroller = NULL;
  #endif
  #ifdef USE_PROLOGIX_DEVICE
    prologixDevice * pdevice = NULL;
  #endif
  #ifdef USE_RATTN_HANDLER
    rattnHandler * rattn = NULL;
  #endif
  #ifdef USE_EEPROM_HANDLER
    eepromHandler * eeprom = NULL;
  #endif
  #ifdef USE_MACRO_HANDLER_PM
    macroHandlerPM * macropm = NULL;
  #endif
  #ifdef USE_DIAGNOSTIC_HELPER
    Diagnostix * diag = NULL;
  #endif
};

#endif    // AR488_MODULES