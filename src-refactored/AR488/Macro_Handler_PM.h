#ifndef MACRO_HANDLER_EP_H
#define MACRO_HANDLER_EP_H

#include "Arduino.h"
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "Utility_Helper.h"



/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                  MACRO HANDLER for PROGMEM                  ||
||                                                             ||
\*=============================================================*/


#ifdef USE_MACRO_HANDLER_PM

/*************************************/
/***** MACRO DEFINITIONS SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/

/*
 * This setion can be edited to add macros.
 * (See the AR488 user manual for details)
 */

/***** Example macro *****
#define MACRO_0 "\
++addr 9\n\
++auto 1\n\
*LLO\n\
:func 'volt:ac'\
"
*************************/


/***** Startup Macro *****/
#define MACRO_0 "\
++ver\n\
++addr 22\n\
++addr\n\
"
/* End of MACRO_0 (Startup macro)*/

/***** User macros 1-9 *****/

#define MACRO_1 "\
++ver\n\
++mode\n\
++addr\n\
"
/*<--End of macro 1*/

#define MACRO_2 "\
"
/*<--End of macro 2*/

#define MACRO_3 "\
"
/*<--End of macro 3*/

#define MACRO_4 "\
"
/*<--End of macro 4*/

#define MACRO_5 "\
"
/*<--End of macro 5*/

#define MACRO_6 "\
"
/*<--End of macro 6*/

#define MACRO_7 "\
"
/*<--End of macro 7*/

#define MACRO_8 "\
"
/*<--End of macro 8*/

#define MACRO_9 "\
"
/*<--End of macro 9*/


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MACRO DEFINITIONS SECTION *****/
/*************************************/




/************************************/
/***** MACRO STRUCTURES SECTION *****/
/*****  !!! DO NOT MODIFY !!!   *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/

//#define MACRO_BUFSIZE 128


/***** STARTUP MACRO *****/
const char startup_macro[] PROGMEM = {MACRO_0};

/***** Consts holding USER MACROS 1 - 9 *****/
const char macro_1 [] PROGMEM = {MACRO_1};
const char macro_2 [] PROGMEM = {MACRO_2};
const char macro_3 [] PROGMEM = {MACRO_3};
const char macro_4 [] PROGMEM = {MACRO_4};
const char macro_5 [] PROGMEM = {MACRO_5};
const char macro_6 [] PROGMEM = {MACRO_6};
const char macro_7 [] PROGMEM = {MACRO_7};
const char macro_8 [] PROGMEM = {MACRO_8};
const char macro_9 [] PROGMEM = {MACRO_9};

/* Macro pointer array */
const char * const macros[] PROGMEM = {
  startup_macro,
  macro_1,
  macro_2,
  macro_3,
  macro_4,
  macro_5,
  macro_6,
  macro_7,
  macro_8,
  macro_9
 };


//#define MACR_FUNC_TYPE_VOID  0x40
#define MACR_FUNC_TYPE_PARAM 0x80


class macroHandlerPM {

  public:

    macroHandlerPM(inputBuffer& inBuf, utilityHelper& utils);
    int execCmd(char *cmd, char * params);
    int getHelp(char * token, bool find);
    size_t available() const;
    char charAt(size_t idx) const;
    void clearMacro();

  private:

    inputBuffer& _inBuf;
    utilityHelper& _util;

    /***** Comand names and descriptions - void functions *****/    
    struct macCmdTxtIdx {
      const char* token;
      const uint8_t idx;
      const char* desc;
    };

    /***** Comand function record - parameter functions *****/
    struct macCmdRecChar { 
      void (macroHandlerPM::*handler)(char *);
    };

    // Struct instance declarations
    static const macCmdTxtIdx macCmdIndex[];
    static const macCmdRecChar macCmdHidxChar[];


    int runCmd(char * cmd, char * params);
    int getHelpInfo(char * token, bool find);

    void macro_h(char *params);
    bool macroExists(uint8_t mnum);
    void listMacros();
    void runMacro_h(long int idx);
    size_t showMacro(long int idx);

    char * _currentMacro;
    size_t _msize;

};

#endif  // USE_MACRO_HANDLER_PM

/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/*****  !!! DO NOT MODIFY !!!   *****/
/***** MACRO STRUCTURES SECTION *****/
/************************************/

#endif  // MACRO_HANDLER_EP_H
