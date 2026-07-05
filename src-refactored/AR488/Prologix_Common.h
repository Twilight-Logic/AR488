#ifndef PROLOGIX_COMMON_H
#define PROLOGIX_COMMON_H

#include <Arduino.h>
//#include <avr/pgmspace.h>
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                 PROLOGIX COMMON FUNCTIONS                   ||
||                                                             ||
\*=============================================================*/


#define PCOM_FUNC_TYPE_VOID  0x40
#define PCOM_FUNC_TYPE_PARAM 0x80

//extern bool isVerb;


#ifdef USE_EEPROM_HANDLER
    #include "EEPROM_Handler.h"
#endif

#ifdef USE_MACRO_HELPER
    #include "Macro_Helper.h"
#endif



class prologixCommon {

    public:

        prologixCommon(GPIBbus &bus, utilityHelper &utilh);

        void init();
        int execCmd(char * cmd, char * params);        
        int getHelp(char * token, bool find);

    private:

        // GPIB bus object
        GPIBbus& gpibBus;

        // Utilities
        utilityHelper& _util;

        // Diagnostics module
//        Diagnostix diag;

        /***** Comand names and descriptions - void functions *****/    
        struct comCmdTxtIdx {
            const char* token;
            const uint8_t idx;
            const char* desc;
        };

        /***** Comand function pointers - void functions *****/
        struct comCmdRecVoid {
            void (prologixCommon::*handler)();
        };

        /***** Comand names and descriptions - parameter functions *****/
/*
        struct comCmdTxtChar { 
            const char* token;
            const char* desc;
        };
*/
        /***** Comand function pointers - parameter functions *****/
        struct comCmdRecChar { 
            void (prologixCommon::*handler)(char *);
        };

        // Struct instance declarations
        static const comCmdTxtIdx comCmdIndex[];
        static const comCmdRecVoid comCmdHidxVoid[];
        static const comCmdRecChar comCmdHidxChar[];


        // Module support functions
        int runCmd(char * cmd, char * params);
        int getHelpInfo(char * token, bool find);

        // Utility functions
        void printFWver();
        void printId(uint8_t idx, bool header);
        void showFlag(const __FlashStringHelper* flag);

        // Prologix compatible command handlers (without parameters)
        void rst_h();

        // Expanded command handlers (with parameters)
        void verb_h();

        // Prologix compatible command handlers (with parameters)
        void addr_h(char *params);
        void cmode_h(char *params);
        void eoi_h(char *params);
        void eos_h(char *params);
        void eot_char_h(char *params);
        void eot_en_h(char *params);
        void rtmo_h(char *params);
        void ver_h(char *params);

        // Expanded command handlers (with parameters)
        void aint_h(char *params);
        void eor_h(char *params);
        void id_h(char *params);

};


#endif	// PROLOGIX_COMMON_H

