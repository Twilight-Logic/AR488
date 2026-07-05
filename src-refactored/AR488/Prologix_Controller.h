#ifndef PROLOGIX_HANDLER_H
#define PROLOGIX_HANDLER_H


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                PROLOGIX CONTROLLER FUNCTIONS                ||
||                                                             ||
\*=============================================================*/


#include <Arduino.h>
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"


#if defined(USE_PROLOGIX_CONTROLLER)


#ifdef USE_EEPROM_HANDLER
    #include "EEPROM_Handler.h"
#endif


/***** Number of elements in function pointer list *****/
#define PCTL_FUNC_TYPE_VOID  0x40
#define PCTL_FUNC_TYPE_PARAM 0x80


class prologixController {

    public:

        prologixController(GPIBbus& bus, utilityHelper& utilh);

        void init();
        void init2();
        int execCmd(char * cmd, char * params);
        int getHelp(char * token, bool find);
        void autorun(bool enabled=false);
        void sendToInstrument(char *buffr, uint8_t dsize);
        void signalBreak();

    private:

        // GPIB bus object
        GPIBbus& gpibBus;

        // Utilities
        utilityHelper& _util;

        // Diagnostics module
//        Diagnostix diag;


        /***** Comand names and descriptions - void functions *****/    
        struct ctlCmdTxtIdx {
            const char* token;
            const uint8_t idx;
            const char* desc;
        };


        /***** Comand function record - void functions *****/
        struct ctlCmdRecVoid {
            void (prologixController::*handler)();
        };


        /***** Comand function record - parameter functions *****/
        struct ctlCmdRecChar {
            void (prologixController::*handler)(char *);
        };

        // Struct instance declarations
        static const ctlCmdTxtIdx ctlCmdIndex[];
        static const ctlCmdRecVoid ctlCmdHidxVoid[];
        static const ctlCmdRecChar ctlCmdHidxChar[];

        unsigned long _autoTime;

        // GPIB data receive flags
        bool autoRead = false;              // Auto reading (auto mode 3) GPIB data in progress
        bool readWithEoi = false;           // Read eoi requested
        bool readWithEndByte = false;       // Read with specified terminator character
        // uint8_t tranBrk = 0;                // Transmission break on 1=++, 2=EOI, 3=ATN 4=UNL
        uint8_t endByte = 0;                // Termination character

        // Data send mode flags
        bool dataBufferFull = false;    // Flag when parse buffer is full

        // SRQ auto mode
        bool isSrqa = false;
    
        // Utility functions
        void runMacro(uint8_t macro);
        void showFlag(const __FlashStringHelper* flag);
        int getHelpInfo(char * token, bool find);

        // Controller functions
        bool isIdnQuery(char *buffr, uint8_t dsize);
        int runCmd(char * cmd, char * params);

        // Prologix compatible command handlers (without parameters)
        void clr_h();
        void dcl_h();
        void ifc_h();

        // Expanded command handlers (with parameters)
        void aspoll_h();
        void ppoll_h();
        void srq_h();
        void unlisten_h();
        void untalk_h();

        // Prologix compatible command handlers (with parameters)
        // Helpers
        uint16_t readFrom(char * params);
        bool validReadEndType(char * param);

        // Handlers
        void amode_h(char *params);
        void llo_h(char *params);
        void loc_h(char *params);
        void read_h(char *params);
        void spoll_h(char *params);
        void trg_h(char *params);

        // Expanded command handlers (with parameters)
        void fndl_h(char *params);
        void hflags_h(char * params);
        void idn_h(char * params);
        void srqa_h(char *params);
        void ren_h(char *params);
        void repeat_h(char *params);
        void send_h(char *params);
        void tct_h(char *params);

};

#endif      //USE_PROLOGIX_HANDLER


#endif  // PROLOGIX_HANDLER_H

