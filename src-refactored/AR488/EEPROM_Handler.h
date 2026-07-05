#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "GPIBbus_Handler.h"
#include "Utility_Helper.h"



/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                        EEPROM HANDLER                       ||
||                                                             ||
\*=============================================================*/


/*
 * EEPROM SIZES:
 * 
 * ATmega2560/1284   4096   // Mega 2560, MightyCore 1284 
 * ATmega644         2048   // MightyCore 644
 * ATmega328/32u4    1024   // Uno, Nano, Leonardo
 * ATmega168          512
 * ATmega4809         256   // Nano Every
 * ESP8266/ESP32     none   // Emulated. 512kb assigned?
 * RASPBERRY PICO       ?
 * TEENSY 4/4.1         ?
 */

#ifdef EEPROM_ENABLED


#define EESIZE 256
#define EESTART 2    // EEPROM start of data - min 4 for CRC32, min 2 for CRC16
#define UPCASE true

#define EEPM_FUNC_TYPE_VOID  0x40
#define EEPM_FUNC_TYPE_PARAM 0x80


const uint16_t eesize = EESIZE;


class eepromHandler {

  public:

    eepromHandler(GPIBbus& bus, utilityHelper& utils);

    int execCmd(char * cmd, char * params);
    int getHelp(char * token, bool find);

    void loadCfg();
    
  private:

    GPIBbus& gpibBus;
    utilityHelper& _util;

    /***** Comand names and descriptions - void functions *****/    
    struct eepCmdTxtIdx {
      const char* token;
      const uint8_t idx;
      const char* desc;
    };

    /***** Comand function record - parameter functions *****/
    struct eepCmdRecVoid { 
      void (eepromHandler::*handler)();
    };

//    static void (eepromHandler::*eepFuncVoid[])();
//    static void (eepromHandler::*eepFuncChar[])(char *);

    /***** Comand function record - parameter functions *****/
    struct eepCmdRecChar { 
      void (eepromHandler::*handler)(char *);
    };

    // Struct instance declarations
    static const eepCmdTxtIdx eepCmdIndex[];
    static const eepCmdRecVoid eepCmdHidxVoid[];
    static const eepCmdRecChar eepCmdHidxChar[];

    int runCmd(char * cmd, char * params);
    int getHelpInfo(char * token, bool find);
//    void printHelpInfo(const __FlashStringHelper * token, const __FlashStringHelper * desc);

    void epErase();
//    void epWriteData(uint8_t cfgdata[], size_t cfgsize);
//    bool epReadData(uint8_t cfgdata[], size_t cfgsize);
    void epViewData(Stream& outputStream);
    bool isEepromClear();

    void default_h(char * params);
    void load_h();
    void save_h();

    #if defined(__AVR__) || defined (ARDUINO_ARCH_RENESAS) || defined(__IMXRT1062__)

      template <typename T> void epWriteData(const T& cfgdata) {
        const byte * p = (const byte*) &cfgdata; 
        uint16_t crc;
 
        // Write data
        for (size_t i=0; i<sizeof(cfgdata); i++){
          EEPROM.update(i+EESTART,*p++);
        }
        // Write CRC
        crc = getCRC16(cfgdata);
        EEPROM.put(0, crc);
      }

      template <typename T> bool epReadData(const T& cfgdata) {
        uint16_t crc1;
        uint16_t crc2;
        byte * p = (byte*) &cfgdata;

        // Read CRC
        EEPROM.get(0,crc1);
        // Read data
        for (size_t i=0; i<sizeof(cfgdata); i++){
          *p++ = EEPROM.read(i+EESTART);
        }
        // Get CRC of config
        crc2 = getCRC16(cfgdata);
        if (crc1==crc2) {
          return true;
        }else{
          return false;
        }
      }

    #endif  // __AVR__) || ARDUINO_ARCH_RENESAS || defined(__IMXRT1062__)


    #if defined(ESP8266) || defined(ESP32)

      template <typename T> void epWriteData(const T& cfgdata) {
        const byte * p = (const byte*) &cfgdata; 
        uint16_t crc;

        // Load EEPROM data from Flash
        EEPROM.begin(EESIZE);
        // Write data
        for (size_t i=0; i<sizeof(cfgdata); i++){
          EEPROM.write(i+EESTART,*p++);
        }
        // Write CRC
        crc = getCRC16(cfgdata);
        EEPROM.put(0, crc);
        // Commit write to Flash
        EEPROM.commit();
        EEPROM.end();

      }

      template <typename T> bool epReadData(const T& cfgdata) {
        uint16_t crc1;
        uint16_t crc2;
        byte * p = (byte*) &cfgdata;

        // Load EEPROM data from Flash
        EEPROM.begin(EESIZE);
        // Read CRC
        EEPROM.get(0,crc1);
        // Read data
        for (size_t i=0; i<sizeof(cfgdata); i++){
          *p++ = EEPROM.read(i+EESTART);
        }
        EEPROM.end();
        // Get CRC of config
        crc2 = getCRC16(cfgdata);
        if (crc1==crc2) {
          return true;
        }else{
          return false;
        }
      }

    #endif  // ESP8266 || ESP32


    template <typename T> uint16_t getCRC16(const T& cfgdata){
      uint8_t x;
      uint16_t crc = 0xFFFF;
      const byte * p = (const byte*) &cfgdata;

      for (size_t idx=0; idx<sizeof(cfgdata); ++idx) {
        x = crc >> 8 ^ *p;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
        p++;
      }
      return crc;
    }


    template <typename T> unsigned long int getCRC32(const T& cfgdata) {

      const unsigned long crc_table[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
      };
      unsigned long crc = ~0L;
      const byte * p = (const byte*) &cfgdata;

      for (size_t idx=0; idx<sizeof(cfgdata); ++idx) {
        crc = crc_table[(crc ^ *p) & 0x0f] ^ (crc >> 4);
        crc = crc_table[(crc ^ (*p >> 4)) & 0x0f] ^ (crc >> 4);
        crc = ~crc;
        p++;
      }
      return crc;
    }

};


#endif    // EEPROM_ENABLED

#endif // EEPROM_HANDLER_H
