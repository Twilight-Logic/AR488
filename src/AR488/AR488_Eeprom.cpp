#include <Arduino.h>
#include <EEPROM.h>
#include "AR488_Eeprom.h"

/***** AR488_Eeprom.cpp, ver. 0.00.05, 27/06/2020 *****/
/*
 * EEPROM functions implementation
 */



/***** Forward declarations of internal functions *****/
uint16_t getCRC16(uint8_t bytes[], uint16_t bsize);
unsigned long int getCRC32(uint8_t bytes[], uint16_t bsize);


/********************************/
/***** AVR EEPROM functions *****/
/********************************/
#ifdef __AVR__

/***** Show all 512 bytes of EEPROM data *****/
/*
void epViewData() {
  uint16_t addr = 0;
  uint8_t dbuf[16];
  char cnt[4]= {'\0'};
  char oct[4] = {'\0'};

  // Read data
  memset(dbuf, 0x00, 16);
  for (addr=0; addr<512; addr=addr+16){
    sprintf(cnt, "%03d", addr);
    dataOut->print(cnt);
    dataOut->print(":");
    EEPROM.get(addr, dbuf);
    for (int i=0; i<16; i++){
      dataOut->print(" ");
      sprintf(oct, "%02X", dbuf[i]);
      dataOut->print(oct);
    }
    dataOut->println();
  }
}
*/

/***** Clear the EEPROM *****/
void epErase() {
  int i = EESIZE;

  // Load EEPROM data from Flash
  for (i=0; i<EESIZE; i++)
    EEPROM.write(i, 0xFF);
}


/***** Write data to EEPROM (with CRC) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
void epWriteData(uint8_t cfgdata[], uint16_t cfgsize) {
  uint16_t crc;
  uint16_t addr = EESTART;
  uint16_t i = 0;
 
  // Write data
  for (i=0; i<cfgsize; i++){
    EEPROM.update(addr+i,cfgdata[i]);
  }
  // Write CRC
  crc = getCRC16(cfgdata, cfgsize);
  EEPROM.put(0, crc);
  // Commit write to Flash
}


/***** Read data from EEPROM (with CRC check) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
bool epReadData(uint8_t cfgdata[], uint16_t cfgsize) {
  uint16_t crc1;
  uint16_t crc2;
//  uint16_t addr = EESTART;
  uint16_t i=0;

  // Read CRC
  EEPROM.get(0,crc1);
  // Read data
  for (i=0;i<cfgsize;i++){
    cfgdata[i] = EEPROM.read(EESTART+i);
  }
//  EEPROM.get(addr, cfgdata);
  // Get CRC of config
  crc2 = getCRC16(cfgdata, cfgsize);
  if (crc1==crc2) {
    return true;
  }else{
    return false;
  }
}


bool isEepromClear(){
  int16_t crc = 0;

  // Load (first 16 bits) data from EEPROM
  EEPROM.get(0, crc);
  // Return result
  if (crc==-1) {
    return true;
  }else{
    return false;
  }
}

#endif

/************************************/
/***** ESP8266 EEPROM functions *****/
/************************************/
#if defined(ESP8266) || defined(ESP32)

/***** Show all 512 bytes of EEPROM data *****/
/*
void epViewData() {
  uint16_t addr = 0;
  uint8_t dbuf[16];
  char cnt[4]= {'\0'};

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Read data
  memset(dbuf, 0x00, 16);
  for (addr=0; addr<512; addr=addr+16){
    sprintf(cnt, "%03d", addr);
    arSerial->print(cnt);
    arSerial->print(":");
    EEPROM.get(addr, dbuf);
    for (int i=0; i<16; i++){
      dataOut->print(" ");
      sprintf(oct, "%02X", dbuf[i]);
      dataOut->print(oct);
    }
  }
  EEPROM.end();
}
*/

/***** Clear the EEPROM *****/
void epErase() {
  int i = EESIZE;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  for (i=0; i<EESIZE; i++)
    EEPROM.write(i, 0xFF);
  EEPROM.commit();
  EEPROM.end();
}


/***** Write data to EEPROM (with CRC) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
void epWriteData(uint8_t cfgdata[], uint16_t cfgsize) {
  uint16_t crc;
  uint16_t addr = EESTART;
  
  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Write data
  EEPROM.put(addr,cfgdata);
  // Write CRC
  crc = getCRC16(cfgdata, cfgsize);
  EEPROM.put(0, crc);
  // Commit write to Flash
  EEPROM.commit();
  EEPROM.end();
}


/***** Read data from EEPROM (with CRC check) *****/
/*
 * addr = EEPROM address
 * cfg = config data union object
 * csize = size of config data object
 */
bool epReadData(uint8_t cfgdata[], uint16_t cfgsize) {
  uint16_t crc1;
  uint16_t crc2;
  uint16_t addr = EESTART;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Read CRC
  EEPROM.get(0,crc1);
  // Read data
  EEPROM.get(addr, cfgdata);
  EEPROM.end();
  // Get CRC of config
  crc2 = getCRC16(cfgdata, cfgsize);
  if (crc1==crc2) {
    return true;
  }else{
    return false;
  }
}


bool isEepromClear(){
  int16_t crc = 0;

  // Load data from EEPROM
  EEPROM.begin(EESIZE);
  // Read data
  EEPROM.get(0, crc);
  EEPROM.end(); 
  // Return result
  if (crc==-1) {
    return true;
  }else{
    return false;
  }
}

#endif



/**********************************/
/***** COMMON FUNCTIONS *****/
/**********************************/


/***** Generate a CRC *****/

unsigned long int getCRC32(uint8_t bytes[], uint16_t bsize) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };
  unsigned long crc = ~0L;
  
  for (uint16_t idx=0; idx<bsize; ++idx) {
    crc = crc_table[(crc ^ bytes[idx]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (bytes[idx] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

uint16_t getCRC16(uint8_t bytes[], uint16_t bsize){
  uint8_t x;
  uint16_t crc = 0xFFFF;

  for (uint16_t idx=0; idx<bsize; ++idx) {
    x = crc >> 8 ^ bytes[idx];
    x ^= x>>4;
    crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
  }
  return crc;
}
