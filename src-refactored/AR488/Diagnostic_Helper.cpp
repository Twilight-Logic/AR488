#include "AR488_Config.h"
#include "Diagnostic_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                      DIAGNOSTIC HELPER                      ||
||                                                             ||
\*=============================================================*/


#ifdef USE_DIAGNOSTIC_HELPER


/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


static const char diagCmdHelpExtended[] PROGMEM = {
  "xdiag:\t\tBus diagnostics (see the doc)\n"
};



Diagnostix::Diagnostix(GPIBbus& bus, utilityHelper& utils) :
                        gpibBus(bus), _util(utils){

}


/***** Execute a command *****/
int Diagnostix::execCmd(char * cmd, char * params) {

  int execStat = -1;

  if (isVerb) dataPort.println();
  execStat = runCmd(cmd, params);

  // Show a prompt on completion?
  if (isVerb) _util.showPrompt();

  return execStat;
}


/***** Pointer to extended commands help *****/
int Diagnostix::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}



/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/


const char cmd01[] PROGMEM = "xdiag";

const char cmd01desc[] PROGMEM = "\t\tBus diagnostics: ++xdiag 0 bits - data bus; ++xdiag 1 bits - control bus";


/***** Array containing index of accepted ++ commands *****/
/*
 * Commands without parameters require casting to a pointer
 * requiring a char* parameter. The functon is called with
 * NULL by the command processor.
 * 
 * Format: token, mode, function_ptr
 * Mode: 1=device; 2=controller; 3=both;
 */
const Diagnostix::dgnCmdTxtIdx Diagnostix::dgnCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | DIAG_FUNC_TYPE_PARAM), cmd01desc }
};


const Diagnostix::dgnCmdRecChar Diagnostix::dgnCmdHidxChar [] = {
  { &Diagnostix::xdiag_h    }
};


/***** Extract command and pass to handler *****/
int Diagnostix::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(dgnCmdIndex)/sizeof(*dgnCmdIndex);

#ifdef DEBUG_DIAG_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nDiagnostic module:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(dgnCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(dgnCmdIndex[i].idx));
      }
    }
//    if (!find || idx) printHelpInfo( (const __FlashStringHelper *) pgm_read_ptr(&(dgnCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(dgnCmdIndex[i].desc)) );
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(dgnCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(dgnCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
int Diagnostix::runCmd(char * cmd, char * params) {

  uint8_t paramCmdCnt = sizeof(dgnCmdHidxChar)/sizeof(*dgnCmdHidxChar);
  uint8_t cmdIdxCnt = sizeof(dgnCmdIndex)/sizeof(*dgnCmdIndex);

#ifdef DEBUG_EEPM_CMD_HANDLER
  DB_PRINT(F("run command: "), cmd);
#endif

  // If parameter is empty then return immediately
  if (!cmd) return -1;
  // If non-printable character on blank line then return immediately
  if (cmd[0] < 0x20) return 0;

  int idx = 0;

  // Find the handler
  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(dgnCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(dgnCmdIndex[i].idx));
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
    if (idx & DIAG_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (Diagnostix::* mpc)(char *);
//     void *mpcx;
//     mpcx = (void*)pgm_read_ptr(&(comCmdHidxChar[idx].handler));
//     memcpy( &mpc, &mpcx, sizeof(mpc) );
    mpc = dgnCmdHidxChar[idx].handler;
//    mpc = eepCmdHidxChar[idx];

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_DIAG_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
/*
  if (idx & DIAG_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (Diagnostix::* mpv)();
//    void *mpvx;
//    mpvx = (void*)pgm_read_ptr(&(comCmdHidxVoid[idx].handler));
//    memcpy( &mpv, &mpvx, sizeof(mpv) );
    mpv = dgnCmdHidxVoid[(idx & 0x3F)].handler;
//    mpv = eepCmdHidxVoid[idx];

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_DIAG_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }
*/

  // No valid command found
  return -1;
}




void Diagnostix::printCurrentPinState(){
  printDbPinout();
  printCtrlPinout();
}


/***** Print the pinout and pin states of the control bus *****/
void Diagnostix::printCtrlPinout(){
  printPin(F("IFC"), IFC_PIN);
  printPin(F("NDAC"), NDAC_PIN);
  printPin(F("NRFD"), NRFD_PIN);
  printPin(F("DAV"), DAV_PIN);
  printPin(F("EOI"), EOI_PIN);
  printPin(F("REN"), REN_PIN);
  printPin(F("SRQ"), SRQ_PIN);
  printPin(F("ATN"), ATN_PIN);
}


/***** Print the pinout and pin states for the data bus *****/
void Diagnostix::printDbPinout(){
  printPin(F("DIO1"), DIO1_PIN);
  printPin(F("DIO2"), DIO2_PIN);
  printPin(F("DIO3"), DIO3_PIN);
  printPin(F("DIO4"), DIO4_PIN);
  printPin(F("DIO5"), DIO5_PIN);
  printPin(F("DIO6"), DIO6_PIN);
  printPin(F("DIO7"), DIO7_PIN);
  printPin(F("DIO8"), DIO8_PIN);
}


/***** Print pin detials *****/
void Diagnostix::printPin(const __FlashStringHelper* pinid, uint8_t pin){
  char line[50];
  char pname[7];
  strcpy_P(pname, (const char PROGMEM *)pinid);
  sprintf( line, "%s: \t[%d] \t%d", pname, pin, digitalRead(pin) );
  DATA_RAW_PRINTLN(line);
}


void Diagnostix::testCtrlBusValue(uint8_t value) {
  setGpibCtrlDir(0xFF, 0xFF); // Set all as outputs
  setGpibCtrlState(value, 0xFF);
  delay(20);
  printCtrlPinout();
  // Reset after 10 seconds
  delay(10000);
  setGpibCtrlState(0x00, 0xFF);
}


void Diagnostix::testDataBusValue(uint8_t value) {
  // Set to required value
  setGpibDbus(value);
  delay(20);
  printDbPinout();
  // Reset after 10 seconds
  delay(10000);
  setGpibDbus(0);
}

/***** Output result in ASCII *****/
/*
 * - text is output in ASCII
 * - ATN commands are shown as hex, e.g. [00]
 *
 * _byteRead:
 *    true = byte has been read
 *    false = flag cleared
 * _atnState:
 *    true = ATN asserted
 *    false = ATN un-asserted
*/
void Diagnostix::asciiOut(){

  int atn = ~digitalRead(ATN_PIN);
  int dav = ~digitalRead(DAV_PIN);

  uint8_t db;

   if (dav && !_byteRead) {
    if (atn) {
      if (!_atnAsserted) {
        _atnAsserted = true;
        Serial.println();
      }
      char x[5] = {'\0'};
      db = readGpibDbus();
      _byteRead = true;
      sprintf(x,"[%02X]", db);
      dataPort.print(x);
    }else{
      if (_atnAsserted) {
        Serial.println();
        _atnAsserted = false;
      }
      db = readGpibDbus();
      _byteRead = true;
      dataPort.write(db);
    }
  }

  if (!dav && _byteRead) _byteRead = false;

}


void Diagnostix::toggleTE(){
  #ifdef SN7516X_TE
    digitalWrite(SN7516X_TE, !digitalRead(SN7516X_TE));
  #else
    dataPort.print(F("Unavailable"));
  #endif
}


void Diagnostix::toggleDC(){
  #ifdef SN7516X_DC
    digitalWrite(SN7516X_DC, !digitalRead(SN7516X_DC));
  #else
    digitalWrite(REN_PIN, !digitalRead(REN_PIN));
  #endif
}


void Diagnostix::toggleSC(){
  #ifdef SN7516X_SC
    digitalWrite(SN7516X_SC, !digitalRead(SN7516X_SC));
  #else
    dataPort.print(F("Unavailable"));
  #endif
}


void Diagnostix::hexOut(){

}


/***** Show state or enable/disable promiscuous mode *****/
/*
void prom_h(char *params) {
  uint16_t pval;
  if (params != NULL) {
    if (notInRange(params, 0, 1, pval)) return;
    isProm = pval ? true : false;
    if (isProm) {
      isTO = 0;     // Talk-only mode must be disabled!
      isRO = false; // Listen-only mode must be disabled!
    }
    if (isVerb) {
      dataPort.print(F("PROM: "));
      dataPort.println(pval ? "ON" : "OFF") ;
    }
  } else {
    dataPort.println(isProm);
  }
}
*/


/***** Bus diagnostics *****/
/*
 * Usage: xdiag mode byte
 * mode: pins=sho state of all pins; 0=data bus; 1=control bus
 * byte: byte to write on the bus
 * Note: values to switch individual bits = 1,2,4,8,10,20,40,80
 * States revert to controller or device mode after 10 seconds
 * Databus reverts to 0 (all HIGH) after 10 seconds
 */
void Diagnostix::xdiag_h(char *params){
  char *param;
  uint8_t mode = 0;
  uint8_t byteval = 0;
  
  // Get first parameter (mode = 0 or 1)
  param = strtok(params, " ,\t");

  if (param != nullptr) {

    if (strncasecmp(param, "pins", 4) == 0) {
      printCurrentPinState();
      return;
    }

    if (strncasecmp(param, "ascii", 4) == 0) {
      gpibBus.setControls(DINI);
      while (true) asciiOut();
      gpibBus.setControls(CIDS);
      return;
    }

    if (strncasecmp(param, "hex", 4) == 0) {
      gpibBus.setControls(DINI);
      hexOut();
      gpibBus.setControls(CIDS);
      return;
    }

    if (strncasecmp(param, "tte", 4) == 0) {
      toggleTE();
      return;
    }

    if (strncasecmp(param, "tdc", 4) == 0) {
      toggleDC();
      return;
    }

    if (strncasecmp(param, "tsc", 4) == 0) {
      toggleSC();
      return;
    }

    if (strlen(param)<4){
      mode = atoi(param);
      if (mode>2) {
        dataPort.println(F("Invalid: 0=data bus; 1=control bus"));
        return;
      }
    }

  }

  // Get second parameter (8 bit byte)
  param = strtok(NULL, " \t");

  if (param != nullptr) {

    if (strlen(param)<4){
      byteval = atoi(param);
    }

    switch (mode) {
      case 0:
          // Test data bus by setting required value
          testDataBusValue(byteval);
          break;
      case 1:
          testCtrlBusValue(~byteval);
          if (gpibBus.cfg.cmode==2) {
            gpibBus.setControls(CINI);
          }else{
            gpibBus.setControls(DINI);
          }
          break;
    }

  }

}

#endif    // USE_DIAGNOSTIC_HELPER







