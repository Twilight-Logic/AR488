#ifndef PROLOGIX_DEVICE_H
#define PROLOGIX_DEVICE_H

#include <Arduino.h>
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                 PROLOGIX DEVICE FUNCTIONS                   ||
||                                                             ||
\*=============================================================*/


#ifdef USE_PROLOGIX_DEVICE

//#define PCOM_FUNC_TYPE_VOID  0x40
#define PDEV_FUNC_TYPE_PARAM 0x80



class prologixDevice {

    public:

        prologixDevice(GPIBbus& bus, utilityHelper& utilh);

        void init();
        int execCmd(char *cmd, char * params);
        int getHelp(char * token, bool find);

        void lonMode(Stream &dataStream);
//        void tonMode(char *buffr, uint8_t dsize);
        void tonMode(Stream &dataStream);
        bool isLonEnabled();
        bool isTonEnabled();

    private:

        // GPIB bus object
        GPIBbus gpibBus;

        // Utilities
        utilityHelper _util;

        /***** Comand names and descriptions - void functions *****/    
        struct devCmdTxtIdx {
            const char* token;
            const uint8_t idx;
            const char* desc;
        };

        /***** Comand function record - parameter functions *****/
        struct devCmdRecChar { 
            void (prologixDevice::*handler)(char *);
        };

        // Struct instance declarations
        static const devCmdTxtIdx devCmdIndex[];
//        static const devCmdRecVoid devCmdHidxVoid[];
        static const devCmdRecChar devCmdHidxChar[];


        // Read only mode flag
        bool _isRO;

        // Talk only mode flag
        bool _isTO;

        int runCmd(char * cmd, char * params);
        int getHelpInfo(char * token, bool find);


        // Prologix compatible command handlers (with parameters)
        void lon_h(char *params);
        void stat_h(char *params);

        // Expanded command handlers (with parameters)
        void ton_h(char *params);

};


#endif  // USE_PROLOGIX_DEVICE

#endif	// PROLOGIX_DEVICE_H

