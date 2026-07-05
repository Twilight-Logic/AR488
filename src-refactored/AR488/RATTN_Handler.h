#ifndef RATTN_HANDLER_H
#define RATTN_HANDLER_H

#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||               PROLOGIX DEVICE MODE FUNCTIONS                ||
||                                                             ||
\*=============================================================*/


#ifdef USE_RATTN_HANDLER

// Function types (void, with parameters)
#define RATN_FUNC_TYPE_VOID  0x40
#define RATN_FUNC_TYPE_PARAM 0x80


// Command mode states
#define GPIBNONE 0
#define GPIBLISTEN 1
#define GPIBTALK 2
#define GPIBBOTH 3


extern bool parserReset;
extern bool parserTalkNow;

class rattnHandler {

    public:
    
        rattnHandler(GPIBbus& bus, inputBuffer& inbuffer, utilityHelper& utilh);

        int execCmd(char * cmdexpr);
        void attnRequired();
        void sdc_h();
        bool resetParser();
        bool readWithEoi();

        #ifdef DEVICE_IDN_TEXT        
          bool sendIdnRequest();
        #endif


    private:

        GPIBbus& gpibBus;
        inputBuffer& _inbuf;
        utilityHelper& _util;

           /***** Comand function record - void functions *****/
        struct ratCmdRecVoid { 
            const char* token;
            void (rattnHandler::*handler)();
        };


        /***** Comand function record - parameter functions *****/
        struct ratCmdRecChar { 
            const char* token; 
            void (rattnHandler::*handler)(char *);
        };

        static const ratCmdRecVoid ratCmdHidxVoid[];
        static const ratCmdRecChar ratCmdHidxChar[];

        // General functions
        int runCmd(char * cmdexpr);
        void execGpibCmd(uint8_t gpibcmd);
        void processATN(uint8_t cmdbytes[], size_t bytecnt, uint8_t atnstat, uint8_t stat);
        void showATNStatus(uint8_t atnstat, uint8_t ustat, uint8_t rstat, uint8_t atnbytes[], size_t bcnt);

        // GPIB command handlers
        void waitForATN();
        void atn_listen_h();
        void atn_talk_h();
        void atn_spd_h();
        void atn_spe_h();
        void atn_sdc_h();

//        void tct_h();
        bool atn_unl_h();
        bool atn_unt_h();

        void fakeListenHandler();


#ifdef REMOTE_SIGNAL_PIN
        void atn_gtl_h();
        void atn_llo_h();
#endif

        uint8_t _isAddressed;   // Addressed state (0x20 = listen; 0x40 = talk)
        bool _isSpoll;          // Serial poll flag
        bool _parserReset;      // Trigger parser reset
        bool _readEoi;          // Read using EOI signel as terminator

};


#endif  // USE_RATTN_HANDLER

#endif  // RATTN_HANDLER_H
