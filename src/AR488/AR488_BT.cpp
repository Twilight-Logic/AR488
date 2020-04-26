#include <Arduino.h>

#include "AR488_BT.h"
#include "AR488_Config.h"

#ifdef AR_BT_EN


/***** AR488_BT.cpp, ver. 0.48.24, 25/04/2020 *****/
/*
* AR488 HC05 BlueTooth module
*/

/***** BT Serial Port *****/
#ifdef AR_CDC_SERIAL
  Serial_ *btSerial = &(AR_SERIAL_PORT);
#endif
#ifdef AR_HW_SERIAL
  HardwareSerial *btSerial = &(AR_SERIAL_PORT);
#endif
// Note: SoftwareSerial support conflicts with PCINT support
#ifdef AR_SW_SERIAL
  #include <SoftwareSerial.h>
  SoftwareSerial *btSerial = &swSerial(AR_SW_SERIAL_RX, AR_SW_SERIAL_TX);
#endif

/*
#ifdef DEBUG9
  HardwareSerial *dbSerial = &Serial3;
#endif
*/

/***** Debug Port *****/
/*
#ifdef DB_SERIAL_PORT
  #ifdef DB_CDC_SERIAL
    Serial_ *dbBtSerial = &(DB_SERIAL_PORT);
  #endif
  #ifdef DB_HW_SERIAL
    HardwareSerial *dbBtSerial = &(DB_SERIAL_PORT);
  #endif
  // Note: SoftwareSerial support conflicts with PCINT support
  #ifdef DB_SW_SERIAL
    #include <SoftwareSerial.h>
    SoftwareSerial swSerial(DB_SW_SERIAL_RX, DB_SW_SERIAL_TX);
    SoftwareSerial *dbBtSerial = &swSerial;
  #endif
#endif
*/

// Buffer
#define RBSIZE 64
char rBuf[RBSIZE];


/******************************/ 
/*****  BLUETOOTH SUPPORT *****/
/******************************/

/***** Initialise *****/
void btInit() {
#ifdef DEBUG9
  dbSerial->begin(115200);
  dbSerial->println(F("BlueTooth Debug =>"));
#endif
  // Enable bluetooth HC05 board config mode
  pinMode(AR_BT_EN, OUTPUT);
  digitalWrite(AR_BT_EN, HIGH);
  // Detect baud rate
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
      // No - then configure
      delay(400);
      // Configure BT
      if (btCfg()) blinkLed(3);
    }
  }

  // Reset bluetooth HC05 board and enter user mode
  delay(2000);
  digitalWrite(AR_BT_EN, LOW);
  delay(500);
  btSerial->print(F("AT+RESET\r\n"));
  delay(500);
  btSerial->end();
#ifdef DEBUG9
  dbSerial->println(F("<= END."));
#endif
}


/***** Detect the HC05 board config mode baud rate *****/
bool detectBaud(){
  long int brate[5] = {9600, 19200, 38400, 57600, 115200};
  uint8_t i = 0;
  while (i<5){
    btSerial->begin(brate[i]);
    delay(400);
    btSerial->println("AT");
    delay(200);
    if (atReply("OK")) return true;
    i++;
  }
  return false;
}


/***** Check configuration for change *****/
bool btChkCfg(){
  
  char baudstr[20];
  char baudrate[8];

  memset(baudstr, '\0', 20);
  memset(baudrate, '\0', 8);

  sprintf(baudrate, "%ld", AR_BT_BAUD);
  strncpy(baudstr, "+UART:", 6);
  strcat(baudstr, baudrate);
  strcat(baudstr, ",1,0");

#ifdef DEBUG9
  dbSerial->println(F("Send: AT+NAME?"));
#endif
  btSerial->println(F("AT+NAME?"));
  delay(200);
  if (!atReply("+NAME:" AR_BT_NAME)) return false;
#ifdef DEBUG9
  dbSerial->println(F("BT Name OK."));
#endif
  delay(200);

#ifdef DEBUG9
  dbSerial->println(F("Send: AT+UART?"));
#endif
  btSerial->println(F("AT+UART?"));
  delay(200);
  if(!atReply(baudstr)) return false;
#ifdef DEBUG9
  dbSerial->println(F("Baud rate OK."));
#endif
  delay(200);
  
#ifdef DEBUG9
  dbSerial->println(F("Send: AT+PSWD?"));
#endif
  btSerial->println(F("AT+PSWD?"));
  delay(200);
  if (!atReply("+PIN:\"" AR_BT_CODE "\"")) return false;
#ifdef DEBUG9
  dbSerial->println(F("BT code OK."));
#endif
  return true;
}


/***** Configure the HC05 *****/
bool btCfg(){
#ifdef DEBUG9
  dbSerial->println(F("Configuring..."));
#endif
  btSerial->println(F("AT+ROLE=0"));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG9
  dbSerial->println(F("Set BT role 0"));
#endif

  btSerial->println(F("AT+NAME=\"" AR_BT_NAME "\""));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG9
  dbSerial->println(F("Set BT name: " AR_BT_NAME));
#endif

  btSerial->println(F("AT+PSWD=\"" AR_BT_CODE "\""));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG9
  dbSerial->println(F("Set BT passcode: " AR_BT_CODE));
#endif

  btSerial->print(F("AT+UART="));
  btSerial->print(AR_BT_BAUD);
  btSerial->println(F(",1,0"));
  delay(200);
  if (!atReply("OK")) return false;
#ifdef DEBUG9
  dbSerial->println(F("Set baud rate: "));
#endif

  return true;
}


/***** Is the reply what we expected? *****/
bool atReply(const char* reply) {
  int sz = strlen(reply);

  memset(rBuf, '\0', RBSIZE);
  // Read the first line
  btSerial->readBytesUntil(0x0A, rBuf, RBSIZE-1);
#ifdef DEBUG9
  dbSerial->print(F("Reply: "));
  dbSerial->println(rBuf);
#endif
  // Discared the rest
  while (btSerial->available()){
    btSerial->read();
  }
  if (strncmp(reply, rBuf, sz) == 0) return true;
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

#endif  // AR_BT_EN
