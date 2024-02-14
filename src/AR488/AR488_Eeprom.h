#ifndef AR488_EEPROM_H
#define AR488_EEPROM_H

#include "AR488_Config.h"
//#include <EEPROM.h>

/***** AR488_Eeprom.h, ver. 0.00.07, 21/12/2023 *****/

/*
 * EEPROM SIZES:
 * 
 * ATmega2560/1284   4096   // Mega 2560, MightyCore 1284 
 * ATmega644         2048   // MightyCore 644
 * ATmega328/32u4    1024   // Uno, Nano, Leonardo
 * ATmega168          512
 * ARmega4809         256   // Nano Every
 * ESP8266/ESP32     none   // Emulated. 512kb assigned?
 */


#define EESIZE 512
#define EESTART 2    // EEPROM start of data - min 4 for CRC32, min 2 for CRC16
#define UPCASE true


const uint16_t eesize = EESIZE;


//  extern Stream& dataStream;
//  extern Stream& debugStream;

//void epViewData();

/*
template<typename T> void epViewData(T* output) {
  uint16_t addr = 0;
  uint8_t dbuf[16];
  char cnt[4]= {'\0'};
  char oct[4] = {'\0'};

  // Read data
  memset(dbuf, 0x00, 16);
  for (addr=0; addr<512; addr=addr+16){
    sprintf(cnt, "%03d", addr);
    output->print(cnt);
    output->print(":");
    EEPROM.get(addr, dbuf);
    for (int i=0; i<16; i++){
      output->print(" ");
      sprintf(oct, "%02X", dbuf[i]);
      output->print(oct);
    }
    output->println();
  }
}
*/


/*
 * EEPROM function declarations
 */


void epErase();
void epWriteData(uint8_t cfgdata[], uint16_t cfgsize);
bool epReadData(uint8_t cfgdata[], uint16_t cfgsize);
void epViewData(Stream& outputStream);
bool isEepromClear();


#endif // AR488_EEPROM_H
