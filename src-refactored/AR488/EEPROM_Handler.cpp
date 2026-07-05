
//#include <avr/interrupt.h>

#include "EEPROM_Handler.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                        EEPROM HANDLER                       ||
||                                                             ||
\*=============================================================*/


#ifdef EEPROM_ENABLED


#if defined(__atmega4809__)
    #include <avr/interrupt.h>
#endif

#include <EEPROM.h>


/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


eepromHandler::eepromHandler(GPIBbus& bus, utilityHelper& utils) : gpibBus(bus), _util(utils) {

}


/***** Execute a command *****/
int eepromHandler::execCmd(char * cmd, char * params) {

  int execStat = -1;

  execStat = runCmd(cmd, params);
  return execStat;

}


/***** Pointer to extended commands help *****/
int eepromHandler::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}


void eepromHandler::loadCfg(){
  load_h();
}


/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/


const char cmd01[] PROGMEM = "default";
const char cmd02[] PROGMEM = "loadcfg";
const char cmd03[] PROGMEM = "savecfg";

const char cmd01desc[] PROGMEM = "\tSet defualt config and optionally clear EEPROM: ++default; ++default wipe";
const char cmd02desc[] PROGMEM = "\tLoad the current configuration from EEPROM";
const char cmd03desc[] PROGMEM = "\t\tSave the current configuratrion to EEPROM";


/***** Array containing index of accepted ++ commands *****/
/*
 * Commands without parameters require casting to a pointer
 * requiring a char* parameter. The functon is called with
 * NULL by the command processor.
 * 
 * Format: token, mode, function_ptr
 * Mode: 1=device; 2=controller; 3=both;
 */
const eepromHandler::eepCmdTxtIdx eepromHandler::eepCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | EEPM_FUNC_TYPE_PARAM), cmd01desc },
  { cmd02, (0x00 | EEPM_FUNC_TYPE_VOID),  cmd02desc },
  { cmd03, (0x01 | EEPM_FUNC_TYPE_VOID),  cmd03desc },
};


/***** Array containing index of accepted ++ commands with parameters*****/
const eepromHandler::eepCmdRecVoid eepromHandler::eepCmdHidxVoid [] = {
  { &eepromHandler::load_h    },
  { &eepromHandler::save_h    }
};


/***** Array containing index of accepted ++ commands with parameters*****/
const eepromHandler::eepCmdRecChar eepromHandler::eepCmdHidxChar [] = {  
  { &eepromHandler::default_h }
};


/***** Extract command and pass to handler *****/
int eepromHandler::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(eepCmdIndex)/sizeof(*eepCmdIndex);

#ifdef DEBUG_EEPM_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nEEPROM handler:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(eepCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(eepCmdIndex[i].idx));
      }
    }
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(eepCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(eepCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
//int prologixCommon::runCmd(char * cmd) {
int eepromHandler::runCmd(char * cmd, char * params) {

  uint8_t voidCmdCnt = sizeof(eepCmdHidxVoid)/sizeof(*eepCmdHidxVoid);
  uint8_t paramCmdCnt = sizeof(eepCmdHidxChar)/sizeof(*eepCmdHidxChar);
  uint8_t cmdIdxCnt = sizeof(eepCmdIndex)/sizeof(*eepCmdIndex);

#ifdef DEBUG_EEPM_CMD_HANDLER
  DB_PRINT(F("command buffer: "), cmd);
#endif

  // If parameter is empty then return immediately
  if (!cmd) return -1;
  // If non-printable character on blank line then return immediately
  if (cmd[0] < 0x20) return 0;

  int idx = 0;

  // Find the handler
  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(eepCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(eepCmdIndex[i].idx));
      break;
    }
  }

  // Handler not found
  if (!idx) {
    // If command not found then return
    #ifdef DEBUG_EEPM_CMD_HANDLER
      DB_PRINT(F("Handler not found!"), "");
    #endif
    return -1;
  }

  // Debug info
  #ifdef DEBUG_EEPM_CMD_HANDLER
    DB_PRINT(F("found handler for: "), cmd);
    DB_PRINT(F("Idx dec: "), idx);
  #endif

  // Run the handler

  // Handler accepts parameters
    if (idx & EEPM_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (eepromHandler::* mpc)(char *);
    mpc = eepCmdHidxChar[idx].handler;

    #ifdef DEBUG_EEPM_CMD_HANDLER
      DB_PRINT(F("calling: "), cmd);
      DB_PRINT(F("params:  "), params);
      DB_HEXB_PRINT(F("params hex: "), params, strlen(params));
    #endif

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_EEPM_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
  if (idx & EEPM_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (eepromHandler::* mpv)();
    mpv = eepCmdHidxVoid[(idx & 0x3F)].handler;

    #ifdef DEBUG_EEPM_CMD_HANDLER
      DB_PRINT(F("calling: "), cmd);
    #endif

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_EEPM_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // No valid command found
  return -1;
}


/***** Re-load default configuration *****/
void eepromHandler::default_h(char *params) {
  if (params != NULL) {
#ifdef EEPROM_ENABLED
    if (strncasecmp(params, "wipe", 4) == 0) {
      epErase();
      dataPort.println(F("EEPROM erased!"));
    }else{
      _util.errorMsg(2);
    }
#else
  void(params);
#endif  // EEPROM_ENABLED
  }
  gpibBus.setDefaultCfg();
}


void eepromHandler::load_h(){
    
  // Read config from EEPROM
  #ifdef DEBUG_EEPROM_MODULE
    DB_RAW_PRINTLN(F("Read config from the EEPROM..."));
  #endif

  // Read data from non-volatile memory
  //(will only read if previous config has already been saved)
  if (!isEepromClear()) {
    #ifdef DEBUG_EEPROM_MODULE
      DB_RAW_PRINTLN(F("EEPROM has data."));
    #endif
//    if (!epReadData(gpibBus.cfg.db, GPIB_CFG_SIZE)) {
    if (!epReadData(gpibBus.cfg)) {
      // CRC check failed - config data does not match EEPROM
      #ifdef DEBUG_EEPROM_MODULE
        DB_RAW_PRINTLN(F("CRC check failed. Erasing EEPROM...."));
      #endif
      epErase();
      #ifdef DEBUG_EEPROM_MODULE
        DB_RAW_PRINTLN(F("Erased!"));
      #endif
      gpibBus.setDefaultCfg();
//      epWriteData(gpibBus.cfg.db, GPIB_CFG_SIZE);
      epWriteData(gpibBus.cfg);
      #ifdef DEBUG_EEPROM_MODULE
        DB_RAW_PRINTLN(F("Initialised with defaults."));
      #endif
    }
  #ifdef DEBUG_EEPROM_MODULE
  }else{
    DB_RAW_PRINTLN(F("EEPROM is clear!"));
  #endif  // E2END
  }
}


/***** Save controller configuration *****/
void eepromHandler::save_h() {
#ifdef EEPROM_ENABLED
//  epWriteData(gpibBus.cfg.db, GPIB_CFG_SIZE);
  epWriteData(gpibBus.cfg);
  if (isVerb) dataPort.println(F("Settings saved."));
#else
  dataPort.println(F("Disabled"));
#endif  // EEPROM_ENABLED
}


/********************************/
/***** AVR EEPROM functions *****/
/********************************/
#if defined (__AVR__) || defined (ARDUINO_ARCH_RENESAS) || defined(__IMXRT1062__)

/***** Show all bytes of EEPROM data *****/


void eepromHandler::epViewData(Stream& outputStream) {
  uint16_t addr = 0;
  uint8_t dbuf[16];
  char cnt[4]= {'\0'};
  char oct[4] = {'\0'};

  // Read data
  memset(dbuf, 0x00, 16);
  for (addr=0; addr<EESIZE; addr=addr+16){
    sprintf(cnt, "%03d", addr);
    outputStream.print(cnt);
    outputStream.print(":");
    EEPROM.get(addr, dbuf);
    for (int i=0; i<16; i++){
      outputStream.print(" ");
      sprintf(oct, "%02X", dbuf[i]);
      outputStream.print(oct);
    }
    outputStream.println();
  }
}


/***** Clear the EEPROM *****/
void eepromHandler::epErase() {
  int i = EESIZE;

  // Load EEPROM data from Flash
  for (i=0; i<EESIZE; i++)
    EEPROM.write(i, 0xFF);
}


bool eepromHandler::isEepromClear(){
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

#endif  // __AVR__


/******************************************/
/***** ESP8266/ESP32 EEPROM functions *****/
/******************************************/
#if defined(ESP8266) || defined(ESP32)

/***** Show all bytes of EEPROM data *****/
void eepromHandler::epViewData(Stream& outputStream) {
  uint16_t addr = 0;
  uint8_t dbuf[16];
  char cnt[4]= {'\0'};
  char oct[4] = {'\0'};


  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  // Read data
  memset(dbuf, 0x00, 16);
  for (addr=0; addr<EESIZE; addr=addr+16){
    sprintf(cnt, "%03d", addr);
    outputStream.print(cnt);
    outputStream.print(":");
    EEPROM.get(addr, dbuf);
    for (int i=0; i<16; i++){
      outputStream.print(" ");
      sprintf(oct, "%02X", dbuf[i]);
      outputStream.print(oct);
    }
  }
  EEPROM.end();
}


/***** Clear the EEPROM *****/
void eepromHandler::epErase() {
  int i = EESIZE;

  // Load EEPROM data from Flash
  EEPROM.begin(EESIZE);
  for (i=0; i<EESIZE; i++)
    EEPROM.write(i, 0xFF);
  EEPROM.commit();
  EEPROM.end();
}


bool eepromHandler::isEepromClear(){
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

#endif    // ESP8266/ESP32



#endif    // EEPROM_ENABLED
