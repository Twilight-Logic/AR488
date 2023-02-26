#ifndef AR488_COMPORTS_H
#define AR488_COMPORTS_H

#include <Arduino.h>
#include "AR488_Config.h"

/***** AR488_ComPorts.cpp, ver. 0.51.18, 26/02/2023 *****/


/***** DEVNULL Library *****
 *  AUTHOR: Rob Tillaart
 *  VERSION: 0.1.5
 *  PURPOSE: Arduino library for a /dev/null stream - useful for testing
 *  URL: https://github.com/RobTillaart/DEVNULL
 */

class DEVNULL : public Stream
{
public:
  DEVNULL();

  int    available();
  int    peek();
  int    read();
  void   flush();  //  placeholder to keep CI happy

  size_t write(const uint8_t data);
  size_t write( const uint8_t *buffer, size_t size);

  int    lastByte();

private:
  uint8_t  _bottomLessPit;
};


/*
 * Serial Port definition
 */


// SoftwareSerial library required?
#if defined(AR_SERIAL_SWPORT) || defined(DB_SERIAL_SWPORT)
  #include <SoftwareSerial.h>
#endif



#ifdef DATAPORT_ENABLE

  extern Stream& dataPort;
  void startDataPort();

  #define DATAPORT_START() startDataPort()
  #define DATA_RAW_PRINT(str) dataPort.print(str)
  #define DATA_RAW_PRINTLN(str) dataPort.println(str)

#else

  extern Stream& dataPort;

  #define DATAPORT_START()
  #define DATA_RAW_PRINT(str)
  #define DATA_RAW_PRINTLN(str)

#endif  // DATAPORT_ENABLE



#ifdef DEBUG_ENABLE

  extern Stream& debugPort;
  void startDebugPort();
  void getFuncName(char * funcstr, const char * function);

  template<typename T1, typename T2>
  void debugPrint(const char * function, const char * filestr, int line, T1 msg1, T2 msg2){
    const char * filename = (strrchr(filestr, '/') ? strrchr(filestr, '/') + 1 : filestr);
//    funcstr[strrchr(funcstr,'(')] = '\0';
//    const char * function = strrchr(funcstr,' ') + 1;
    dataPort.print(filename);
    dataPort.print(':');
    dataPort.print(line);
    dataPort.print(" (");
    dataPort.print(function);
    dataPort.print(") > ");
    dataPort.print(msg1);
    dataPort.println(msg2);
  }

  void printHex(uint8_t byteval);
  void printHexArray(uint8_t barray[], size_t asize);
  void printHexBuf(char * buf, size_t bsize);

  #define DEBUG_START() startDebugPort()
  #define DB_PRINT(msg1,msg2) debugPrint(__FUNCTION__, __FILE__, __LINE__,(msg1),(msg2))
  #define DB_RAW_PRINT(msg) debugPort.print(msg)
  #define DB_RAW_PRINTLN(msg) debugPort.println(msg)
  #define DB_HEX_PRINT(byteval) printHex(byteval)
  #define DB_HEXA_PRINT(msg, barray, bsize) debugPort.print(msg);printHexArray(barray, bsize)
  #define DB_HEXB_PRINT(msg, buf, bsize) debugPort.print(msg);printHexBuf(buf, bsize)


#else

  #define DEBUG_START()
  #define DB_PRINT(msg1,msg2)
  #define DB_RAW_PRINT(msg)
  #define DB_RAW_PRINTLN(msg)
  #define DB_HEX_PRINT(byteval)
  #define DB_HEXB_PRINT(buf, bsize)

#endif  // DEBUG_ENABLE


/***** BlueTooth Functions *****/

#ifdef AR_SERIAL_BT_ENABLE

//  #define BTRBSIZE 64

  const size_t BTrbufSize = 64;

  void btInit();
  void blinkLed(uint8_t count);
  bool btChkCfg();
  bool btCfg();
  bool detectBaud();
  bool atReply(const char* reply);

#endif


#endif  // AR488_COMPORTS_H
