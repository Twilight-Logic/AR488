#include <Arduino.h>
#include "AR488_ComPorts.h"

/***** AR488_ComPorts.cpp, ver. 0.51.18, 26/02/2023 *****/


/***** DEVNULL Library *****
 *  AUTHOR: Rob Tillaart
 *  VERSION: 0.1.5
 *  PURPOSE: Arduino library for a /dev/null stream - useful for testing
 *  URL: https://github.com/RobTillaart/DEVNULL
 */

DEVNULL::DEVNULL()
{
  setTimeout(0);        //  no timeout.
  _bottomLessPit = -1;  //  nothing in the pit
}

int  DEVNULL::available()
{
  return 0;
};

int  DEVNULL::peek()
{
  return EOF;
};

int  DEVNULL::read()
{
  return EOF;
};

//  placeholder to keep CI happy
void DEVNULL::flush()
{
  return;
};

size_t DEVNULL::write(const uint8_t data)
{
  _bottomLessPit = data;
  return 1;
}

size_t DEVNULL::write( const uint8_t *buffer, size_t size)
{
  if (size > 0) _bottomLessPit = buffer[size - 1];
  return size;
}

int DEVNULL::lastByte()
{
  return _bottomLessPit;
}



/***************************************/
/***** Serial Port implementations *****/
/***************************************/


/****************************/ 
/***** DATA SERIAL PORT *****/
/****************************/

#ifdef DATAPORT_ENABLE
  #ifdef AR_SERIAL_SWPORT

    SoftwareSerial dataPort(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);

    void startDataPort() {
      dataPort.begin(AR_SERIAL_SPEED);
    }

  #else

    Stream& dataPort = AR_SERIAL_PORT;

    void startDataPort() {
      AR_SERIAL_PORT.begin(AR_SERIAL_SPEED);
    }
  
  #endif

#else

  DEVNULL _dndata;
  Stream& dataPort = _dndata;

#endif  // DATAPORT_ENABLE



/*****************************/ 
/***** DEBUG SERIAL PORT *****/
/*****************************/

#ifdef DEBUG_ENABLE
  #ifdef DB_SERIAL_SWPORT

    SoftwareSerial debugPort(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);

    void startDebugPort() {
      debugPort.begin(DB_SERIAL_SPEED);
    }
  
  #else

    Stream& debugPort = DB_SERIAL_PORT;

    void startDebugPort() {
      DB_SERIAL_PORT.begin(DB_SERIAL_SPEED);
    }

  #endif

  void printHex(uint8_t byteval) {
    char x[4] = {'\0'};
    sprintf(x,"%02X ", byteval);
    debugPort.print(x);
  }

  void printHexBuf(char * buf, size_t bsize){
    for (size_t i = 0; i < bsize; i++) {
      printHex(buf[i]);
    }
    debugPort.println();
  }
  
  void printHexArray(uint8_t barray[], size_t asize){
    for (size_t i = 0; i < asize; i++) {
      printHex(barray[i]);
    }
    debugPort.println();
  }

#else

  DEVNULL _dndebug;
  Stream& debugPort = _dndebug;

#endif  // DEBUG_ENABLE




/**************************/ 
/***** BLUETOOTH PORT *****/
/**************************/

#ifdef AR_SERIAL_BT_NAME

  char BTrbuf[BTrbufSize];

/***** Initialise *****/
void btInit() {
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("BlueTooth Debug =>"));
  delay(100);
#endif
  // Enable bluetooth HC05 board config mode
  pinMode(AR_SERIAL_BT_ENABLE, OUTPUT);
  digitalWrite(AR_SERIAL_BT_ENABLE, HIGH);

  // Detect baud rate (this will check variious baud rates - serial may display gibberish)
  if (detectBaud()) {
    // Baud rate detected
    blinkLed(2);
    delay(400);

    // Are we already configured?
    if (btChkCfg()) {
      // Yes - blink LED once
      delay(400);
      blinkLed(1);  
    }else{
      // No - then configure - set speed to AR_SERIAL_SPEED
      delay(400);
      // Configure BT
      if (btCfg()) blinkLed(3);
    }

  }

  // Reset bluetooth HC05 board and enter user mode
  delay(2000);
  digitalWrite(AR_SERIAL_BT_ENABLE, LOW);
  delay(500);
  dataPort.print(F("AT+RESET\r\n"));
  delay(500);

  // Reset baud rate to AR_SERIAL_SPEED
#ifdef AR_SERIAL_SWPORT
//  AR_SERIAL_SWPORT.end();
//  delay(100);
  AR_SERIAL_SWPORT.begin(AR_SERIAL_SPEED);
#else
//  AR_SERIAL_PORT.end();
//  delay(100);
  AR_SERIAL_PORT.begin(AR_SERIAL_SPEED);
#endif

#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("<= END."));
#endif

}


/***** Detect the HC05 board config mode baud rate *****/
bool detectBaud(){
  long int brate[5] = {9600, 19200, 38400, 57600, 115200};
  uint8_t i = 0;
  while (i<5){
#ifdef AR_SERIAL_SWPORT
    AR_SERIAL_SWPORT.begin(brate[i]);
#else
    AR_SERIAL_PORT.begin(brate[i]);
#endif
    delay(400);
    dataPort.println("AT");
    delay(200);
    if (atReply("OK")) return true;
    i++;
  }
  return false;
}


/***** Check configuration for change *****/
/*
 * To see the debug output from this function, the Terminal must initially be set 
 * to the native baud rate of the HC05. Initially, this will probably be 9600 or 
 * 38400. Once successfully configured, the baud rate of the HC05 should then match 
 * the baud rate set in AR_SERIAL_SPEED and the output will be visible with the 
 * Terminal set to AR_SERIAL_SPEED.
 */
bool btChkCfg(){
  
  char baudstr[20];
  char baudrate[8];

  memset(baudstr, '\0', 20);
  memset(baudrate, '\0', 8);

  sprintf(baudrate, "%ld", AR_SERIAL_SPEED);
  strncpy(baudstr, "+UART:", 6);
  strcat(baudstr, baudrate);
  strcat(baudstr, ",1,0");

#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Send: AT+NAME?"));
#endif
  dataPort.println(F("AT+NAME?"));
  delay(200);
  if (!atReply("+NAME:" AR_SERIAL_BT_NAME)) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("BT Name OK."));
#endif
  delay(200);

#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Send: AT+UART?"));
#endif
  dataPort.println(F("AT+UART?"));
  delay(200);
  if(!atReply(baudstr)) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Baud rate OK."));
#endif
  delay(200);
  
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Send: AT+PSWD?"));
#endif
  dataPort.println(F("AT+PSWD?"));
  delay(200);
  if (!atReply("+PIN:\"" AR_SERIAL_BT_CODE "\"")) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("BT code OK."));
#endif
  return true;
}


/***** Configure the HC05 *****/
bool btCfg(){
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Configuring..."));
#endif
  dataPort.println(F("AT+ROLE=0"));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Set BT role 0"));
#endif

  dataPort.println(F("AT+NAME=\"" AR_SERIAL_BT_NAME "\""));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Set BT name: " AR_SERIAL_BT_NAME));
#endif

  dataPort.println(F("AT+PSWD=\"" AR_SERIAL_BT_CODE "\""));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Set BT passcode: " AR_SERIAL_BT_CODE));
#endif

  dataPort.print(F("AT+UART="));
  dataPort.print(AR_SERIAL_SPEED);
  dataPort.println(F(",1,0"));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Set baud rate: "));
#endif

  return true;
}


/***** Is the reply what we expected? *****/
bool atReply(const char* reply) {
  int sz = strlen(reply);

  memset(BTrbuf, '\0', BTrbufSize);
  // Read the first line
  dataPort.readBytesUntil(0x0A, BTrbuf, BTrbufSize-1);
#ifdef DEBUG_BLUETOOTH
  DB_RAW_PRINTLN(F("Reply: "));
  DB_RAW_PRINTLN(BTrbuf);
#endif
  // Discared the rest
  while (dataPort.available()){
    dataPort.read();
  }
  if (strncmp(reply, BTrbuf, sz) == 0) return true;
  return false;
}


/***** Blink the internal LED *****/
void blinkLed(uint8_t count){
  for (uint8_t i=0; i<count; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

/*************************************/ 
/*****  ENF OF BLUETOOTH SUPPORT *****/
/*************************************/

#endif
