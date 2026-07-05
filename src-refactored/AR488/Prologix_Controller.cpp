#include <Arduino.h>
#include "AR488_Config.h"
#include "Prologix_Controller.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                PROLOGIX CONTROLLER FUNCTIONS                ||
||                                                             ||
\*=============================================================*/


#if defined(USE_PROLOGIX_CONTROLLER)


/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


prologixController::prologixController(GPIBbus& bus, utilityHelper& utilh) :
                                        gpibBus(bus), _util(utilh) {

}


void prologixController::init() {
  gpibBus.startControllerMode();
  if (gpibBus.cfg.hflags & 0x01) showFlag(F("AR488~RDY"));
}



void prologixController::init2() {

  isVerb = false;
/*
  // Start the interface in the configured mode
  if (gpibBus.cfg.cmode == 2) {
    gpibBus.startControllerMode();
    if (gpibBus.cfg.hflags == 0xFF) gpibBus.cfg.hflags = 0;
  }else if (gpibBus.cfg.cmode == 1){
    gpibBus.startDeviceMode();
  }
*/

/***** Run the startup macro if enabled *****/
#if defined(USE_MACRO_HELPER) && defined(RUN_STARTUP_MACRO)
  runMacro(0);
#endif

}


/***** Execute a command *****/
int prologixController::execCmd(char * cmd, char * params) {

  int execStat = -1;

  execStat = runCmd(cmd, params);
  return execStat;

}


/***** Pointer to extended commands help *****/
int prologixController::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}


/***** Auto run functions *****/
void prologixController::autorun(bool enabled) {
  unsigned long currentTime = millis();
  if (enabled){
    if ( currentTime > (_autoTime + gpibBus.cfg.aint) ) {
      if ( (gpibBus.cfg.amode==3) && autoRead ) {
        gpibBus.addressDevice(gpibBus.cfg.paddr, gpibBus.cfg.saddr, TOTALK);
        gpibBus.receiveData(dataPort, readWithEoi, readWithEndByte, endByte);
        gpibBus.unAddressDevice();
      }
      _autoTime = millis();
    }
  }else{
    autoRead = false;
  }
}





/****** Send data to instrument *****/
/* Processes the parse buffer when full or CR/LF detected
 * and sends data to the instrument
 */
void prologixController::sendToInstrument(char *buffr, uint8_t dsize) {

//  char eoi[] = "eoi";
  bool isquery = false;
  bool isidnquery = false;
  enum receiveState rstate;

#ifdef DEBUG_SEND_TO_INSTR
  if (buffr[dsize] != LF) DB_RAW_PRINTLN();
  DB_HEXB_PRINT(F("Received for sending: "), buffr, dsize);
#endif

  // Is this an instrument query command (string ending with ?)
  if (gpibBus.cfg.amode == 2) {
    if (buffr[dsize-1] == '?') isquery = true;
  }

  // Is this an *idn? query?
  if (gpibBus.cfg.idn) isidnquery = isIdnQuery(buffr, dsize);

  if (isidnquery && gpibBus.cfg.idn) {

    DATA_RAW_PRINTLN();
    if (gpibBus.cfg.idn == 1) DATA_RAW_PRINTLN(gpibBus.cfg.sname);
    if (gpibBus.cfg.idn == 2) {
      DATA_RAW_PRINT(gpibBus.cfg.sname);
      DATA_RAW_PRINT('-');
      DATA_RAW_PRINTLN(gpibBus.cfg.serial);
    }

  }else{

    if (gpibBus.isController()) {
      // Has controller already addressed the device? - if not then address it
      if (gpibBus.haveAddressedDevice() != TOLISTEN) gpibBus.addressDevice(gpibBus.cfg.paddr, gpibBus.cfg.saddr, TOLISTEN);
    }

    // Send string to instrument
    gpibBus.sendData(buffr, dsize, gpibBus.cfg.eoi);

    // Show handshake flag
    if (gpibBus.cfg.hflags & 0x04) showFlag(F("Send^OK"));

    // Auto-read data from GPIB bus following any command
    if ( (gpibBus.cfg.amode == 1) || isquery ) {
//    if (gpibBus.cfg.amode & 0x03) {
      gpibBus.addressDevice(gpibBus.cfg.paddr, gpibBus.cfg.saddr, TOTALK);
      rstate = gpibBus.receiveData(dataPort, gpibBus.cfg.eoi, false, 0);
      if ( (rstate == (RECEIVE_EOI | RECEIVE_ENDCHAR | RECEIVE_ENDL)) && (gpibBus.cfg.hflags & 0x02) ) showFlag(F("Read^OK"));
      gpibBus.unAddressDevice();
    }

    // If controller then unaddress device
    if (gpibBus.isController() &&  dataBufferFull == false) {
      gpibBus.unAddressDevice();
    }

    // Clear buffer full flag
    if (dataBufferFull) dataBufferFull = false;

    #ifdef DEBUG_SEND_TO_INSTR
      DB_PRINT(F("done."),"");
    #endif

    // Reply to query
//    if ( isquery && (gpibBus.cfg.amode == 2) ) read_h(eoi);

  }

  // Show a prompt on completion?
  if (isVerb) _util.showPrompt();

}


/************** ^^^^^^^^^^^^^^^^ ***************/
/* <----------  PUBLIC FUNCTIONS   ----------> */
/***********************************************/




/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/




/*****************************/
/***** UTILITY FUNCTIONS *****/
/***** vvvvvvvvvvvvvvvvv *****/


void prologixController::showFlag(const __FlashStringHelper* flag){
  uint8_t cnt = (gpibBus.cfg.hflags & 0xE0) >> 5;
  if (!cnt) cnt = 1;
  dataPort.println();
  while (cnt) {
    dataPort.print(flag);
    if (cnt>1) dataPort.print('%');
    cnt--;
  }
  dataPort.println();
}


#ifdef USE_MACRO_HELPER
void prologixController::runMacro(uint8_t macro){
  const size_t bufsize = 48;
  char buffer[bufsize];
  size_t bytecnt = 0;
  while ((bytecnt = macros.readMacro(buffer, bufsize, macro))) {
    if (isVerb) DATA_RAW_PRINTLN(buffer);
    if ( (buffer[0]==0x2B) && (buffer[1]==0x2B) ) {
      execCmd(buffer, bytecnt);
    }else{
      sendToInstrument(buffer, bytecnt);
    }
  }
}
#endif


void prologixController::signalBreak(){
  gpibBus.signalBreak();
}


/***** ^^^^^^^^^^^^^^^^^ *****/
/***** UTILITY FUNCTIONS *****/
/*****************************/



/********************************/
/***** CONTROLLER FUNCTIONS *****/
/***** vvvvvvvvvvvvvvvvvvvv *****/

const char cmd01[] PROGMEM = "allspoll";
const char cmd02[] PROGMEM = "auto";
const char cmd03[] PROGMEM = "clr";
const char cmd04[] PROGMEM = "dcl";
const char cmd05[] PROGMEM = "flags";
const char cmd06[] PROGMEM = "fndl";
const char cmd07[] PROGMEM = "idn";
const char cmd08[] PROGMEM = "ifc";
const char cmd09[] PROGMEM = "llo";
const char cmd10[] PROGMEM = "loc";
const char cmd11[] PROGMEM = "ppoll";
const char cmd12[] PROGMEM = "read";
const char cmd13[] PROGMEM = "ren";
const char cmd14[] PROGMEM = "repeat";
const char cmd15[] PROGMEM = "trg";
const char cmd16[] PROGMEM = "send";
const char cmd17[] PROGMEM = "spoll";
const char cmd18[] PROGMEM = "srqauto";
const char cmd19[] PROGMEM = "tct";
const char cmd20[] PROGMEM = "unt";
const char cmd21[] PROGMEM = "unl";


const char cmd01desc[] PROGMEM = "\tSerial poll all instruments (alias: ++spoll all)";
const char cmd02desc[] PROGMEM = "\t\tAutomatically request talk and read response";
const char cmd03desc[] PROGMEM = "\t\tSend Selected Device Clear to current GPIB address";
const char cmd04desc[] PROGMEM = "\t\tSend unaddressed (all) device clear  [power on reset] (is the rst?)";
const char cmd05desc[] PROGMEM = "\tDisplay handhsaking flags - bits 0 1 & 2 control Ready, ReadOk and SendOK";
const char cmd06desc[] PROGMEM = "\t\tFind listners : 0 - 30, 'all' or range eg. 3-7";
const char cmd07desc[] PROGMEM = "\t\tEnable/Disable reply to *idn? : 0=disabled; 1=enabled";
const char cmd08desc[] PROGMEM = "\t\tAssert IFC signal for 150 miscoseconds - make AR488 controller in charge";
const char cmd09desc[] PROGMEM = "\t\tLocal lockout - disable front panel operation on instrument";
const char cmd10desc[] PROGMEM = "\t\tEnable front panel operation on instrument";
const char cmd11desc[] PROGMEM = "\t\tConduct a parallel poll";
const char cmd12desc[] PROGMEM = "\t\tRead data from instrument: ++read; ++read eoi; ++read @5; ++read @5,eoi";
const char cmd13desc[] PROGMEM = "\t\tAssert or Unassert the REN signal";
const char cmd14desc[] PROGMEM = "\tRepeat a given command and return result";
const char cmd15desc[] PROGMEM = "\t\tSend trigger to selected devices (up to 15 addresses";
const char cmd16desc[] PROGMEM = "\t\tSend data or command to a primary or secondary address: ++send 5,\"*idn?\"";
const char cmd17desc[] PROGMEM = "\t\tSerial poll the addressed host or all instruments";
const char cmd18desc[] PROGMEM = "\tAutomatically conduct serial poll when SRQ is asserted";
const char cmd19desc[] PROGMEM = "\t\tSignal remote device to take control";
const char cmd20desc[] PROGMEM = "\t\tUnlisten the GPIB bus";
const char cmd21desc[] PROGMEM = "\t\tUntalk the GPIB bus";


/***** Command index *****/
/*
 * Index field -
 *    Bit 7 (0x80) = command with parameters  
 *    Bit 6 (0x40) = command without parameters
 *    Bits 0-5 = command index
 *
 * Index allows 32 commands of each type to be mapped
 * Index value MUST correspond to position of pointer to handler
 * cmdCmdHidxVoid and comCmdHidxChar strucs.
*/
const prologixController::ctlCmdTxtIdx prologixController::ctlCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | PCTL_FUNC_TYPE_VOID),  cmd01desc },
  { cmd02, (0x00 | PCTL_FUNC_TYPE_PARAM), cmd02desc },
  { cmd03, (0x01 | PCTL_FUNC_TYPE_VOID),  cmd03desc },
  { cmd04, (0x02 | PCTL_FUNC_TYPE_VOID),  cmd04desc },
  { cmd05, (0x01 | PCTL_FUNC_TYPE_PARAM), cmd05desc },
  { cmd06, (0x02 | PCTL_FUNC_TYPE_PARAM), cmd06desc },
  { cmd07, (0x03 | PCTL_FUNC_TYPE_PARAM), cmd07desc },
  { cmd08, (0x03 | PCTL_FUNC_TYPE_VOID),  cmd08desc },
  { cmd09, (0x04 | PCTL_FUNC_TYPE_PARAM), cmd09desc },
  { cmd10, (0x05 | PCTL_FUNC_TYPE_PARAM), cmd10desc },
  { cmd11, (0x04 | PCTL_FUNC_TYPE_VOID),  cmd11desc },
  { cmd12, (0x06 | PCTL_FUNC_TYPE_PARAM), cmd12desc },
  { cmd13, (0x07 | PCTL_FUNC_TYPE_PARAM), cmd13desc },
  { cmd14, (0x08 | PCTL_FUNC_TYPE_PARAM), cmd14desc },
  { cmd15, (0x09 | PCTL_FUNC_TYPE_PARAM), cmd15desc },
  { cmd16, (0x0A | PCTL_FUNC_TYPE_PARAM), cmd16desc },
  { cmd17, (0x0B | PCTL_FUNC_TYPE_PARAM), cmd17desc },
  { cmd18, (0x0C | PCTL_FUNC_TYPE_PARAM), cmd18desc },
  { cmd19, (0x0D | PCTL_FUNC_TYPE_PARAM), cmd19desc },
  { cmd20, (0x05 | PCTL_FUNC_TYPE_VOID),  cmd20desc },
  { cmd21, (0x06 | PCTL_FUNC_TYPE_VOID),  cmd21desc }
};


/***** Arrays containing index of accepted ++ commands *****/
/*
 * Format: token, function_ptr
 */

const prologixController::ctlCmdRecVoid prologixController::ctlCmdHidxVoid [] = {
  { &prologixController::aspoll_h    },
  { &prologixController::clr_h       },
  { &prologixController::dcl_h       },
  { &prologixController::ifc_h       },
  { &prologixController::ppoll_h     },
  { &prologixController::unlisten_h  },
  { &prologixController::untalk_h    },
};

const prologixController::ctlCmdRecChar prologixController::ctlCmdHidxChar [] = {
  { &prologixController::amode_h     },
  { &prologixController::hflags_h    },
  { &prologixController::fndl_h      },
  { &prologixController::idn_h       },
  { &prologixController::llo_h       },
  { &prologixController::loc_h       },
  { &prologixController::read_h      },
  { &prologixController::ren_h       },
  { &prologixController::repeat_h    },
  { &prologixController::trg_h       },
  { &prologixController::send_h      },
  { &prologixController::spoll_h     },
  { &prologixController::srqa_h      },
  { &prologixController::tct_h       }
};


/***** Is this an *idn? query? *****/
bool prologixController::isIdnQuery(char * buffer, uint8_t dsize) {
  // Check for upper or lower case *idn?
  if (dsize>3){
    if (strncasecmp(buffer, "*idn?", 5)==0) {
#ifdef DEBUG_PARSER
      DB_PRINT(F("isIdnQuery: Detected IDN query."),"");
#endif
      return true;
    }
  }
  return false;
}


/***** Extract command and pass to handler *****/
int prologixController::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(ctlCmdIndex)/sizeof(*ctlCmdIndex);

#ifdef DEBUG_PCOM_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nController module:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(ctlCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(ctlCmdIndex[i].idx));
      }
    }
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(ctlCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(ctlCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
int prologixController::runCmd(char * cmd, char * params) {

  uint8_t voidCmdCnt = sizeof(ctlCmdHidxVoid)/sizeof(*ctlCmdHidxVoid);
  uint8_t paramCmdCnt = sizeof(ctlCmdHidxChar)/sizeof(*ctlCmdHidxChar);

  uint8_t cmdIdxCnt = sizeof(ctlCmdIndex)/sizeof(*ctlCmdIndex);

#ifdef DEBUG_PCTL_CMD_HANDLER
  DB_PRINT(F("command buffer: "), cmd);
#endif

  // If parameter is empty then return immediately
  if (!cmd) return -1;
  // If non-printable character on blank line then return immediately
  if (cmd[0] < 0x20) return 0;

  int idx = 0;

  // Find the handler
  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(ctlCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(ctlCmdIndex[i].idx));
      break;
    }
  }

  // Handler not found
  if (!idx) {
    // If command not found then return
    #ifdef DEBUG_PCTL_CMD_HANDLER
      DB_PRINT(F("Handler not found!"), "");
    #endif
    return -1;
  }

  // Debug info
  #ifdef DEBUG_PCTL_CMD_HANDLER
    DB_PRINT(F("found handler for: "), cmd);
    DB_PRINT(F("Idx dec: "), idx);
  #endif

  // Run the handler

  // Handler accepts parameters
    if (idx & PCTL_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (prologixController::* mpc)(char *);
//     void *mpcx;
//     mpcx = (void*)pgm_read_ptr(&(comCmdHidxChar[idx].handler));
//     memcpy( &mpc, &mpcx, sizeof(mpc) );
    mpc = ctlCmdHidxChar[idx].handler;

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_PCTL_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
  if (idx & PCTL_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (prologixController::* mpv)();
//    void *mpvx;
//    mpvx = (void*)pgm_read_ptr(&(comCmdHidxVoid[idx].handler));
//    memcpy( &mpv, &mpvx, sizeof(mpv) );
    mpv = ctlCmdHidxVoid[(idx & 0x3F)].handler;

    #ifdef DEBUG_PCTL_CMD_HANDLER
      DB_PRINT(F("calling: "), cmd);
    #endif

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_PCTL_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // No valid command found
  return -1;
}


/******^^^^^^^^^^^^^^^^^^^^******/
/***** CONTROLLER FUNCTIONS *****/
/********************************/



/************************************************/
/***** PROLOGIX COMPATIBLE COMMAND HANDLERS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/


/***** Show or enable/disable auto mode *****/
void prologixController::amode_h(char *params) {
  uint16_t val;
  if (params) {
    if (_util.notInRange(params, 0, 3, val)) return;
    if (val > 0 && isVerb) {
      dataPort.println(F("WARNING: automode ON can cause some devices to generate"));
      dataPort.println(F("         'addressed to talk but nothing to say' errors"));
    }
    gpibBus.cfg.amode = (uint8_t)val;
    if (gpibBus.cfg.amode < 3) autoRead = false;
/*
    if (isVerb) {
      dataPort.print(F("Auto mode: "));      // 3rd parameter
      dataPort.println(gpibBus.cfg.amode);
    }
*/    
  } else {
    dataPort.println(gpibBus.cfg.amode);
  }
}


/***** Send device clear (usually resets the device to power on state) *****/
//void prologixController::clr_h(char * params) {
void prologixController::clr_h() {
//  params = NULL;
  if (gpibBus.sendSDC())  {
    if (isVerb) dataPort.println(F("Failed to send SDC"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Assert IFC for 150 microseconds *****/
/* This indicates that the AR488 the Controller-in-Charge on
 * the bus and causes all interfaces to return to their idle 
 * state
 */
//void prologixController::ifc_h(char * params) {
void prologixController::ifc_h() {
//  params = NULL;    // Ignore any passed parameters
  if (gpibBus.cfg.cmode==2) {
    // Assert IFC
    gpibBus.assertSignal(IFC_BIT);
    delayMicroseconds(150);
    // De-assert IFC
    gpibBus.clearSignal(IFC_BIT);
    if (isVerb) dataPort.println(F("IFC signal asserted for 150 microseconds"));
  }
}


/***** Send local lockout command *****/
void prologixController::llo_h(char * params) {
  // NOTE: REN *MUST* be asserted (LOW)
  if (digitalRead(REN_PIN)==LOW) {
    // For 'all' send LLO to the bus without addressing any device
    // Devices will show REM as soon as they are addressed and need to be released with LOC
    if (params != NULL) {
      if (0 == strncasecmp(params, "all", 3)) {
        if (gpibBus.sendCmd(GC_LLO)) {
          if (isVerb) dataPort.println(F("Failed to send universal LLO."));
        }
      }
    } else {
      // Send LLO to currently addressed device
      if (gpibBus.sendLLO()){
        if (isVerb) dataPort.println(F("Failed to send LLO!"));
      }
    }
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Send Go To Local (GTL) command *****/
void prologixController::loc_h(char * params) {
  // REN *MUST* be asserted (LOW)
  if (digitalRead(REN_PIN)==LOW) {
    if (params != NULL) {
      if (strncasecmp(params, "all", 3) == 0) {
        // Send request to clear all devices to local
        gpibBus.sendAllClear();
      }
    } else {
      // Send GTL to addressed device
      if (gpibBus.sendGTL()) {
        if (isVerb) dataPort.println(F("Failed to send LOC!"));
      }
      // Set GPIB controls back to idle state
      gpibBus.setControls(CIDS);
    }
  }
}


/***** Determine and set end charcter or EOI signal *****/
bool prologixController::validReadEndType(char * param){
  bool endflg = false;
  uint16_t endval = 0xff;
  if (param){
    if ( (strncasecmp(param, "eoi", 3)) == 0 ){
      readWithEoi = true;
      return true;
    } else if (strlen(param)==1) {
      if (param[0] == '0') {
        endval = 0;
        endflg = true;
      }
    } else if (strlen(param) < 5)  {
      endval = strtoul(param, NULL, 0);
      if (endval) {
        if (endval < 256) endflg = true;
      } else if ( (strncasecmp(param, "0x0", 3) == 0) ) {
        endval = 0;
        endflg = true;
      } 
    }
  }
  if (endflg) {
    endByte = endval;
    readWithEndByte = true;
    return true;
  }
  return false;
}


/***** Read from a specific address *****/
uint16_t prologixController::readFrom(char * params) {
  char * endtype = NULL;
  char * pristr = NULL;
  char * secstr = NULL;
  uint8_t pri = 0xFF;
  uint8_t sec = 0xFF;

  if (params[0]){
    char * addrs = strtok(params, ":");
    if (addrs) {
      endtype = strtok(NULL, ":");
      pristr = strtok(addrs, ", \t");
      if (pristr) {
        secstr = strtok(NULL, ", \t");
        pri = strtoul(pristr, NULL, 0);
        if (pri>31) return 0xFFFF;
      }
      if (secstr) {
        sec = strtoul(secstr, NULL, 0);
        if (sec<31) sec = sec + 0x60;
        if (sec<0x60 || sec>0x7E) return 0xFFFF;
      }
      if (endtype) {
        if (!validReadEndType(endtype)) return 0xFFFF;
      }
      return (sec << 8) | pri;
    }
  }
  return 0xFFFF;
}


/***** Address device to talk and read the sent data *****/
void prologixController::read_h(char *params) {
  uint8_t pri = gpibBus.cfg.paddr;
  uint8_t sec = gpibBus.cfg.saddr;
  uint16_t val = 0xFF;
//  char * param;

  // Clear read flags (Global vars)
  readWithEoi = false;
  readWithEndByte = false;
  endByte = 0;

  if (params) {
    if (params[0] == '@') {
      val = readFrom(params+1);
      if (val == 0xFFFF) {
        _util.errorMsg(2);
        return;
      }else{
        pri = (uint8_t)val&0xFF;
        sec = (uint8_t)(val>>8);
      }
    }else{
      if (!validReadEndType(params)) {
        _util.errorMsg(2);
        return;
      }
    }
  }

  // Address device to talk
  if (gpibBus.haveAddressedDevice() != TOTALK) gpibBus.addressDevice(pri, sec, TOTALK);

  // Read data
  if (gpibBus.cfg.amode == 3) {
    // In auto continuous mode we set this flag to indicate we are ready for continuous read
    autoRead = true;
    _autoTime = millis();
  } else {
    // If auto mode is disabled we do a single read
    gpibBus.receiveData(dataPort, readWithEoi, readWithEndByte, endByte);
    if (gpibBus.cfg.hflags & 0x02) showFlag(F("Read^OK"));
    gpibBus.unAddressDevice();
  }

}


/***** Serial Poll Handler *****/
void prologixController::spoll_h(char *params) {
  char *param;
  const uint8_t acnt = 32;
  uint8_t addrs[acnt] = {0};
  uint8_t sb = 0;
  enum gpibHandshakeState state;
  uint8_t j = 0;
  uint16_t addrval = 0;
  bool all = false;
  bool eoiDetected = false;

  // Read parameters
  if (params == NULL) {   // No parameters - trigger addressed device only
    addrs[0] = gpibBus.cfg.paddr;
    j = 1;
  } else if (strncasecmp(params, "all", 3) == 0) {   // ALL parameter given
    all = true;
    j = 30;
    if (isVerb) dataPort.println(F("Serial poll of all devices requested..."));
  }

  if (j == 0) {

    // Read address parameters into array
    while (j < acnt) {

      if (j == 0) {
        param = strtok(params, " \t");
      } else {
        param = strtok(NULL, " \t");
      }

      // No further parameters so exit 
      if (!param) break;
      
      // Contains only digits
      if (!_util.isNumber(param)) {
        _util.errorMsg(2);
        return;
      }

      // Valid GPIB address parameter length?
      if (strlen(param) > 2) {
        _util.errorMsg(2);
        return;
      }

      // Valid GPIB address parameter ?
      if (_util.notInRange(param, 1, 30, addrval)) return;

      // All good
      addrs[j] = (uint8_t)addrval;
      j++;

    }

  }

  // Send Unlisten [UNL] to all devices
  if ( gpibBus.sendCmd(GC_UNL) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send UNL"),"");
#endif
    return;
  }

  // Controller addresses itself as listner
  if ( gpibBus.sendCmd(GC_LAD + gpibBus.cfg.caddr) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send LAD"),"");
#endif
    return;
  }

  // Send Serial Poll Enable [SPE] to all devices
  if ( gpibBus.sendCmd(GC_SPE) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send SPE"),"");
#endif
    return;
  }

  // Poll GPIB address or addresses as set by i and j
  for (int i = 0; i < j; i++) {

    // Set GPIB address in val
    if (all) {
      addrval = i;
    } else {
      addrval = addrs[i];
    }

    // Don't need to poll own address
    if (addrval != gpibBus.cfg.caddr) {

      // Address a device to talk
      if ( gpibBus.sendCmd(GC_TAD + addrval) )  {

#ifdef DEBUG_SPOLL
        DB_PRINT(F("failed to send TAD"),"");
#endif
        return;
      }

      // Set GPIB control to controller active listner state (ATN unasserted), clear databus and set to input
      gpibBus.setControls(CLAS);
  //    gpibBus.clearDataBus();

      // Read the response byte (usually device status) using handshake - suppress EOI detection
      state = gpibBus.readByte(&sb, false, &eoiDetected);

      // If we successfully read a byte
      if (state == HANDSHAKE_COMPLETE) {

        // Set GPIB control back to controller active talk state (ATN asserted) 
        gpibBus.setControls(CTAS);

        // Process response
        if (j == acnt) {
          // If all, return specially formatted response: SRQ:addr,status
          // but only when RQS bit set
          if (sb & 0x40) {
            dataPort.print(F("SRQ:")); dataPort.print(i); dataPort.print(F(",")); dataPort.println(sb, DEC);
            // Exit on first device to respond
            i = j;
          }
        } else {
          // Return decimal number representing status byte
          dataPort.println(sb, DEC);
          if (isVerb) {
            dataPort.print(F("Received status byte ["));
            dataPort.print(sb);
            dataPort.print(F("] from device at address: "));
            dataPort.println(addrval);
          }
          // Exit on first device to respond
          i = j;
        }
      } else {
        if (isVerb) {
          dataPort.print(F("Failed to retrieve status byte from "));
          dataPort.println(addrval);
        }
      }
    }
  }
  if (all) dataPort.println();

  // Send Serial Poll Disable [SPD] to all devices
  if ( gpibBus.sendCmd(GC_SPD) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send SPD"),"");
#endif
    return;
  }

  // Send Untalk [UNT] to all devices
  if ( gpibBus.sendCmd(GC_UNT) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send UNT"),"");
#endif
    return;
  }

  // Unadress listners [UNL] to all devices
  if ( gpibBus.sendCmd(GC_UNL) )  {
#ifdef DEBUG_SPOLL
    DB_PRINT(F("failed to send UNL"),"");
#endif
    return;
  }

  // Set GPIB control to controller idle state
  gpibBus.setControls(CIDS);

  if (isVerb) dataPort.println(F("Serial poll completed."));

}


/***** Return status of SRQ line *****/
void prologixController::srq_h() {
  //NOTE: LOW=asserted=true=1, HIGH=unasserted=false=0
  dataPort.println(gpibBus.isAsserted(SRQ_PIN));
}


/***** Send a trigger command *****/
void prologixController::trg_h(char *params) {
  const uint8_t maxparam = 32;
  char *param;
  uint8_t addrs[maxparam] = {0};
  uint16_t val = 0;
  uint8_t cnt = 0;

  addrs[0] = addrs[0]; // Meaningless as both are zero but defaults compiler warning!

  // Read parameters
  if (params == NULL) {
    // No parameters - trigger addressed device only
    addrs[0] = gpibBus.cfg.paddr;
    cnt++;
  } else {
    // Read address parameters into array
    while (cnt < maxparam) {
      if (cnt == 0) {
        param = strtok(params, " \t");
      } else {
        param = strtok(NULL, " \t");
      }
      if (param == NULL) {
        break;  // Stop when there are no more parameters
      }else{    
        if (_util.notInRange(param, 1, 30, val)) return;
        addrs[cnt] = (uint8_t)val;
        cnt++;
      }
    }
  }

  // If we have some addresses to trigger....
  if (cnt > 0) {
    for (int i = 0; i < cnt; i++) {
      // Sent GET to the requested device
      if (gpibBus.sendGET(addrs[i]))  {
        if (isVerb) dataPort.println(F("Failed to trigger device!"));
        return;
      }
    }

    // Set GPIB controls back to idle state
    gpibBus.setControls(CIDS);

    if (isVerb) dataPort.println(F("Group trigger completed."));
  }
}


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PROLOGIX COMPATIBLE COMMAND HANDLERS *****/
/************************************************/



/*************************************/
/***** EXTENDED COMMAND HANDLERS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/


/***** All serial poll *****/
/*
 * Polls all devices, not just the currently addressed instrument
 * This is an alias wrapper for ++spoll all
 */
void prologixController::aspoll_h() {
  spoll_h((char*)"all");
}


/***** Send Universal Device Clear *****/
/*
 * The universal Device Clear (DCL) is unaddressed and affects all devices on the Gpib bus.
 */
void prologixController::dcl_h() {
  if ( gpibBus.sendCmd(GC_DCL) )  {
    if (isVerb) dataPort.println(F("Sending DCL failed"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Fine all listeners *****/
void prologixController::fndl_h(char *params) {
  char *param;
  const uint8_t amax = 32;
  uint16_t addrval = 0;
  uint8_t addrList[amax] = {0};
  uint16_t tmo = gpibBus.cfg.rtmo;
  uint8_t acnt = 0;
  uint8_t i = 0;
  uint8_t j = 0;
  uint8_t pri = 0xFF;
  unsigned long range[2] = {0,0};
  bool list = false;

  // Initialise arrays
//  for (int i = 0; i < 15; i++) {
//    addrList[i] = 0;
//  }

  // Set minimal timeout
  gpibBus.cfg.rtmo = 35;

  // Read parameters
  if (params == nullptr) {
    j = amax; // Same as 'all'
  }else{
    // Is it a range?
    if ( _util.isRange(params, strlen(params), range) ) {
      if (range[0]<30 && range[1]<31) {
        i = (uint8_t)range[0];
        j = (uint8_t)range[1] + 1;
      }else{
        _util.errorMsg(2);
        return;
      }
    }
    // Requested 'all'?
    if ( strncasecmp(params, "all", 4) == 0) {
      j = amax;
    }
  }

  if (j==0) {
    // Read address parameters into array
    while (j < amax) {
      if (j == 0) {
        param = strtok(params, " ,\t");
      } else {
        param = strtok(NULL, " ,\t");
      }

      // No further parameters so exit 
      if (!param) break;

      // Valid GPIB address parameter ?
      if (strlen(param) > 2) {
        _util.errorMsg(2);
        return;
      }

      // Contains only digits
      if (!_util.isNumber(param)) {
        _util.errorMsg(2);
        return;
      }

      // Valid range
      if (_util.notInRange(param, 0, 31, addrval)) return;
      addrList[j] = (uint8_t)addrval;
      j++;

    }

    list = true;

  }

  // Poll the range of GPIB adresses
  while (i<j) {

    // Get from list or use actual value of iterator?
    if (list) {
      pri = addrList[i];
    }else{
      pri = i;
    }

    // Ignore the controller address
    if (pri == gpibBus.cfg.caddr) {
      i++;
      continue;
    }

    // Send UNL + UNT + LAD (addressDevice function adds 0x20 to pri)
    if (gpibBus.addressDevice(pri, 0xFF, TOLISTEN) > 0) {
      _util.errorMsg(3);
      break;
    }

    gpibBus.clearSignal(ATN_BIT);
    delayMicroseconds(1600);

    if (gpibBus.isAsserted(NDAC_PIN)) {
 
      if (acnt>0) Serial.print(',');
      dataPort.print(pri);
      acnt++;

    }else{

      // Send all secondary addresses
      gpibBus.assertSignal(ATN_BIT);
      for (uint8_t sec=0x60; sec<0x7F; sec++){
        gpibBus.writeByte(sec, false);
      }
      
      gpibBus.clearSignal(ATN_BIT);
      delayMicroseconds(1600);

      if (gpibBus.isAsserted(NDAC_PIN)) {
        gpibBus.assertSignal(ATN_BIT);
        gpibBus.writeByte(GC_UNL, false);
        gpibBus.writeByte( (pri+0x20), false ); // LAD

        for (uint8_t sec=0x60; sec<0x7F; sec++){
          gpibBus.writeByte(sec, false);
          gpibBus.clearSignal(ATN_BIT);
          delayMicroseconds(1600);
          if (gpibBus.isAsserted(NDAC_PIN)) {
            if (acnt>0) dataPort.print(',');
            acnt++;
            dataPort.print(pri);
            dataPort.print(':');
            dataPort.print(sec);

            gpibBus.assertSignal(ATN_BIT);
            gpibBus.writeByte(GC_UNL, false);
            gpibBus.writeByte((pri+0x20), false); // LAD
          }else{
            gpibBus.assertSignal(ATN_BIT);
            gpibBus.writeByte(GC_UNT, false);
          }
        }

        gpibBus.clearSignal(ATN_BIT);
        delayMicroseconds(1600);

      }

    } // End if NDAC aserted (else)

    gpibBus.setControls(CIDS);
    i++;

  } // END while

  dataPort.println();
  gpibBus.cfg.rtmo = tmo;
  gpibBus.setControls(CIDS);

}


/***** Handshaking indicator flags *****/
/*
 * flags & 0x01 = AR488~RDY
 * flass & 0x02 = Read^OK
 * flags & 0x04 = Send^OK
 */
void prologixController::hflags_h(char * params) {
  uint16_t val = 0;
  uint16_t cnt = 0;
  char * param;
  if (params != NULL) {
    param = strtok(params, " ,\t");
    if (_util.notInRange(param, 0, 7, val)) return;
    if (val) {
      param = strtok(NULL, " ,\t");
      if (param) {
        if (_util.notInRange(param, 1, 7, cnt)) return;
      }else{
        cnt = 1;
      }
    }
    gpibBus.cfg.hflags = (uint8_t)val + ((uint8_t)cnt<<5) ;
  }else{
    dataPort.print(gpibBus.cfg.hflags & 0x1F);
    if (gpibBus.cfg.hflags > 0x1F) {
      dataPort.print(',');
      dataPort.print(gpibBus.cfg.hflags >> 5);
    }
    dataPort.println();
  }
}


void prologixController::idn_h(char * params){
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 2, val)) return;
    gpibBus.cfg.idn = (uint8_t)val;
    if (isVerb) {
      dataPort.print(F("Sending IDN: "));
      dataPort.print(val ? "Enabled" : "Disabled"); 
      if (val==2) dataPort.print(F(" with serial number"));
      dataPort.println();
    };
  } else {
    dataPort.println(gpibBus.cfg.idn, DEC);
  }  
}


/***** Parallel Poll Handler *****/
void prologixController::ppoll_h() {
  uint8_t sb = 0;

  // Poll devices
  // Start in controller idle state
  gpibBus.setControls(CIDS);
  delayMicroseconds(20);

  // Assert ATN and EOI
  gpibBus.setTransmitMode(TM_SEND);
  gpibBus.assertSignal( ATN_BIT | EOI_BIT );
  gpibBus.setTransmitMode(TM_RECV);
  delayMicroseconds(20);

  // Read data byte from GPIB bus without handshake
  sb = readGpibDbus();

  // Return to controller idle state (ATN and EOI unasserted)
  gpibBus.setControls(CIDS);

  // Output the response byte
  dataPort.println(sb, DEC);

  if (isVerb) dataPort.println(F("Parallel poll completed."));
}


/***** Assert or de-assert REN 0=de-assert; 1=assert *****/
void prologixController::ren_h(char *params) {
#if defined (SN7516X) && not defined (SN7516X_DC)
  params = params;
  dataPort.println(F("Unavailable")) ;
#else
  // char *stat;
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 1, val)) return;
//    val ? gpibBus.assertSignal(REN_PIN) | gpibBus.clearSignal(REN_PIN);
    digitalWrite(REN_PIN, (val ? LOW : HIGH));
/*
    if (isVerb) {
      dataPort.print(F("REN: "));
      dataPort.println(val ? "REN asserted" : "REN un-asserted") ;
    };
*/    
  } else {
    dataPort.println(digitalRead(REN_PIN) ? 0 : 1);
  }
#endif
}


/***** Repeatedly send a given instrument instrution and return result *****/
/*
 * Parameters:
 *  count (2 - 255)
 *  time-delay between send and receive (0 - 30,000 ms)
 *  command
*/
void prologixController::repeat_h(char *params) {

  uint16_t count;
  uint16_t tmdly;
  char *param;

  if (params != NULL) {
//Serial.println(F("Repeating!"));
    // Count (number of repetitions)
    param = strtok(params, ", \t");
    if (strlen(param) > 0) {
      if (_util.notInRange(param, 1, 255, count)) return;
    }
    // Time delay (milliseconds)
    param = strtok(NULL, ", \t");
    if (strlen(param) > 0) {
      if (_util.notInRange(param, 0, 30000, tmdly)) return;
    }

    // Pointer to remainder of parameters string
    param = strtok(NULL, "\0");
    if (strlen(param) > 0) {
      for (uint16_t i = 0; i < count; i++) {
        // Send string to instrument
        if (gpibBus.haveAddressedDevice() != TOLISTEN) gpibBus.addressDevice(gpibBus.cfg.paddr, gpibBus.cfg.saddr, TOLISTEN);
        gpibBus.sendData(param, strlen(param), gpibBus.cfg.eoi);
        delay(tmdly);
        gpibBus.addressDevice(gpibBus.cfg.paddr, gpibBus.cfg.saddr, TOTALK);
        gpibBus.receiveData(dataPort, gpibBus.cfg.eoi, gpibBus.cfg.eot_en, gpibBus.cfg.eot_ch);
      }
      gpibBus.unAddressDevice();
    } else {
      _util.errorMsg(1);
//      if (isVerb) dataPort.println(F("Missing parameter"));
      return;
    }
  } else {
    _util.errorMsg(1);
//    if (isVerb) dataPort.println(F("Missing parameters"));
  }
}


/***** Send to secondary address *****/
/*
  Parameters: pri,sec,data
  pri, sec = GPIB addresses between 0 and 30
  data is optional
*/
void prologixController::send_h(char *params) {
  char * param;
  uint8_t pri = 0xFF;
  uint8_t sec = 0xFF;
  uint16_t val;
  enum receiveState rstate;
  bool isquery = false;

  if (params != nullptr) {
    // 1st parameter (must be an address value)
    param = strtok(params, " ,\t");

    // Are addresses numerical
    if (!_util.isNumber(param)) {
      _util.errorMsg(2);
      return;
    }

    // Primary address in range ?
    val = strtoul(param, NULL, 10);
    if (val>30) {
      _util.errorMsg(2);
      return;
    }

    // Not using controller address ?
    if (val == gpibBus.cfg.caddr) {
      _util.errorMsg(2);
      return;
    }

    pri = (uint8_t)val;

    // 2nd parameter (secondary address value or data)
    param = strtok(NULL, " ,\t");
    size_t plen = strlen(param);
    if (!plen) {
      _util.errorMsg(2);
      return;
    }

    if (_util.isNumber(param)) {
      // Secondary address in range ?
      val = strtoul(param, NULL, 10);
      if (val<31) val = val + 0x60;
      if (val<0x60 || val>0x7E) {
        _util.errorMsg(2);
        return;
      }
      sec = (uint8_t)val;

      // 3rd parameter
      param = strtok(NULL, "\0");

    }

    if (gpibBus.cfg.amode == 2) {
      if (param[plen-1] == '?') isquery = true;
    }

    gpibBus.addressDevice(pri, sec, TOLISTEN);
    gpibBus.sendData(param, strlen(param), gpibBus.cfg.eoi);

    if ( (gpibBus.cfg.amode == 1) || isquery ) {
      gpibBus.addressDevice(pri, sec, TOTALK);
      rstate = gpibBus.receiveData(dataPort, gpibBus.cfg.eoi, false, 0);
      if ( (rstate == (RECEIVE_EOI | RECEIVE_ENDCHAR | RECEIVE_ENDL)) && (gpibBus.cfg.hflags & 0x02) ) showFlag(F("Read^OK"));
//      if (isQuery) isQuery = false;
      gpibBus.unAddressDevice();
    }

  }else{
    _util.errorMsg(1);
  }

}


/***** SRQ auto - show or enable/disable automatic spoll on SRQ *****/
/*
 * When SRQ auto is set to 1 and a decivce triggers thw SRQ
 * signal, a serial poll is conducted automatically and
 * the status byte for the instrument requiring service gets
 * returned automatically. When srqauto is set to 0 (default)
 * an ++spoll command needs to be given manually to return
 * the status byte.
 */
void prologixController::srqa_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 1, val)) return;
    switch (val) {
      case 0:
        isSrqa = false;
        break;
      case 1:
        isSrqa = true;
        break;
    }
//    if (isVerb) dataPort.println(isSrqa ? "SRQ auto ON" : "SRQ auto OFF") ;
  } else {
    dataPort.println(isSrqa);
  }
}


/***** Take Control command *****/
void prologixController::tct_h(char *params){
  uint16_t val;
  bool tctfail = false;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 30, val)) return;
    if (val == gpibBus.cfg.caddr) {
      _util.errorMsg(2);
      if (isVerb) dataPort.println(F("That is my address! Please provide the address of a remote device."));
      return;
    }

    tctfail = gpibBus.sendTCT(val);

    if (isVerb) {
      dataPort.print(F("Sending TCT to device at address "));
      dataPort.print(val);
    }
    
    if (tctfail) {
      if (isVerb) dataPort.println(F(" failed!"));
      return;
    }else{
      if (isVerb) dataPort.println(F(" succeeded."));
      gpibBus.startDeviceMode();
      if (isVerb) dataPort.println(F("Switched to device mode."));
    }
  }
}


/***** Send device clear (usually resets the device to power on state) *****/
void prologixController::unlisten_h() {
  if (gpibBus.sendUNL())  {
    if (isVerb) dataPort.println(F("Failed to send UNL"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Send device clear (usually resets the device to power on state) *****/
void prologixController::untalk_h() {
  if (gpibBus.sendUNT())  {
    if (isVerb) dataPort.println(F("Failed to send UNT"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** EXTENDED COMMAND HANDLERS *****/
/*************************************/


/*************** ^^^^^^^^^^^^^^^^^ ***************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************************************************/



#endif    // USE_PROLOGIX_CONTROLLER







