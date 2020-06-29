#ifndef AR488_EEPROM_H
#define AR488_EEPROM_H

#include "AR488_Config.h"
#include <EEPROM.h>

/***** AR488_Eeprom.h, ver. 0.00.05, 27/06/2020 *****/
/*
 * EEPROM function definitions
 */

#define EESIZE 512
#define EESTART 2    // EEPROM start of data - min 4 for CRC32, min 2 for CRC16
#define UPCASE true


//void epViewData();


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


void epErase();
void epWriteData(uint8_t cfgdata[], uint16_t cfgsize);
bool epReadData(uint8_t cfgdata[], uint16_t cfgsize);
bool isEepromClear();


#endif // AR488_EEPROM_H
