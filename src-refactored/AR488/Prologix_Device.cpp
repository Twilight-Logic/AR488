#include <Arduino.h>
#include "AR488_Config.h"
#include "Prologix_Device.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                 PROLOGIX DEVICE FUNCTIONS                   ||
||                                                             ||
\*=============================================================*/


#ifdef USE_PROLOGIX_DEVICE


/***** HELP reference *****/
/*
static const char devCmdHelpPrologix[] PROGMEM = {
  "lon:\t\tPut controller in listen-only mode (listen to all traffic)\n"
  "status:\tSet the status byte to be returned on being polled (bit 6 = RQS, i.e SRQ asserted)\n"
};
static const char devCmdHelpExtended[] PROGMEM = {
  "ton:\t\tPut controller in talk-only mode (send data only)\n"
};
*/

/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


prologixDevice::prologixDevice(GPIBbus& bus, utilityHelper& utilh) :
                        gpibBus(bus), _util(utilh){

  _isRO = false;
  _isTO = false;
}


void prologixDevice::init() {
  gpibBus.startDeviceMode();
}


/***** Execute a command *****/
int prologixDevice::execCmd(char * cmd, char * params) {

  int execStat = -1;

  execStat = runCmd(cmd, params);
  return execStat;

}


/***** Pointer to extended commands help *****/
int prologixDevice::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}


/***** Return LON status *****/
bool prologixDevice::isLonEnabled(){
  return _isRO;
}


/***** Return TON status *****/
bool prologixDevice::isTonEnabled(){
  return _isTO;
}


/***** Run Listen Only (LON) mode *****/
void prologixDevice::lonMode(Stream &dataStream){
  uint8_t db = 0;
  bool eoiDetected = false;
  enum gpibHandshakeState hstate = HANDSHAKE_NOT_READY;
//  enum receiveState rstate = RECEIVE_INIT;
  bool ndacState = GPIB_UNASSERTED;

  if (_isRO) {

    gpibBus.setControls(DLAS);
//    state[0] = _util.waitForPinState(NRFD_PIN, GPIB_ASSERTED, gpibBus.cfg.rtmo);
    ndacState = _util.waitForPinState(NDAC_PIN, GPIB_ASSERTED, gpibBus.cfg.rtmo);

//    Serial.print(F("NRFD: "));
//    Serial.print(digitalRead(NRFD_PIN));
//    Serial.print(F("\tNDAC: "));
//    Serial.println(digitalRead(NDAC_PIN));

//    if (state[0] && state[1]) {
    if (ndacState == GPIB_ASSERTED) {
//      Serial.println(F("Ready."));

//      do {
          hstate = gpibBus.readByte(&db, false, &eoiDetected);
          if (hstate == HANDSHAKE_COMPLETE) dataStream.write(db);
//      } while (hstate == HANDSHAKE_COMPLETE);

//Serial.println(db, HEX);
//Serial.print(F("Hstate: "));
//Serial.println(hstate);
/*
      while (hstate == HANDSHAKE_COMPLETE) {
        hstate = gpibBus.readByte(&db, false, &eoiDetected);
        if (hstate == HANDSHAKE_COMPLETE) dataStream.write(db);
      }
*/
    } else {
//      Serial.println(F("Not ready!"));
      return;
    }


//    rstate = gpibBus.receiveData(dataStream, gpibBus.cfg.eoi, false, 0);

//    enum gpibHandshakeState hstate = gpibBus.readByte(&db, false, &eoiDetected);
//Serial.print(F("Hstate: "));
//Serial.println(hstate);



//    gpibBus.setControls(DINI);

  }
}


/***** Run Talk only (TON) mode *****/
//void prologixDevice::tonMode(char *buffr, uint8_t dsize){
void prologixDevice::tonMode(Stream &dataStream){
  // Set bus for device taker active mode
  gpibBus.setControls(DTAS);
  while (_isTO) {
//    for (uint8_t i=0; i<dsize; i++){
//     gpibBus.writeByte(buffr[i], NO_EOI);  // False = No EOI
//    }
    if (dataStream.available()) {
      char c = dataStream.read();
//      dataStream.write(c);
      gpibBus.writeByte(c, false);
    }
  }
  // Set bus to idle
//  gpibBus.setControls(DIDS);
}


/************** ^^^^^^^^^^^^^^^^ ***************/
/* <----------  PUBLIC FUNCTIONS   ----------> */
/***********************************************/




/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/


const char cmd01[] PROGMEM = "lon";
const char cmd02[] PROGMEM = "status";
const char cmd03[] PROGMEM = "ton";

const char cmd01desc[] PROGMEM = "\t\tPut controller in listen-only mode (listen to all traffic)";
const char cmd02desc[] PROGMEM = "\tSet the status byte to be returned on being polled (bit 6 = RQS, i.e SRQ asserted)";
const char cmd03desc[] PROGMEM = "\t\tPut controller in talk-only mode (send data only)";

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
const prologixDevice::devCmdTxtIdx prologixDevice::devCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | PDEV_FUNC_TYPE_PARAM), cmd01desc },
  { cmd02, (0x01 | PDEV_FUNC_TYPE_PARAM), cmd02desc },
  { cmd03, (0x02 | PDEV_FUNC_TYPE_PARAM), cmd03desc },
};


/***** Array containing index of accepted ++ commands with parameters*****/
const prologixDevice::devCmdRecChar prologixDevice::devCmdHidxChar [] = {  
  { &prologixDevice::lon_h   },
  { &prologixDevice::stat_h  },
  { &prologixDevice::ton_h   }
};


/***** Show state or enable/disable listen only mode *****/
void prologixDevice::lon_h(char * params) {
  uint16_t lval;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 1, lval)) return;
    _isRO = lval ? true : false;
    if (_isRO) {
      _isTO = 0;       // Talk-only mode must be disabled!
      // Set bus for device listner active mode
      gpibBus.setControls(DLAS);
    }else{
      // Set bus to idle
      gpibBus.setControls(DIDS);
    }
//    if (isVerb) {
//      dataPort.print(F("LON: "));
//      dataPort.println(lval ? "ON" : "OFF") ;
//    }
  } else {
    dataPort.println(_isRO);
  }
}


/***** Set the status byte (device mode) *****/
void prologixDevice::stat_h(char *params) {
  uint16_t statusByte = 0;
  // A parameter given?
  if (params != NULL) {
    // Byte value given?
    if (_util.notInRange(params, 0, 255, statusByte)) return;
    gpibBus.setStatus((uint8_t)statusByte);
  } else {
    // Return the currently set status byte
    dataPort.println(gpibBus.cfg.stat);
  }
}


/***** Talk only mode *****/
void prologixDevice::ton_h(char *params) {
  uint16_t toval;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 2, toval)) return;
    _isTO = (uint8_t)toval;
    if (_isTO>0) {
      _isRO = false;   // Read-only mode must be disabled in TO mode!
//      _isProm = false; // Promiscuous mode must be disabled in TO mode!
      // Set bus for device taker active mode
      gpibBus.setControls(DTAS);
    }else{
      // Set bus to idle
      gpibBus.setControls(DIDS);
    }
  }else{
    if (isVerb) {
      DATA_RAW_PRINT(F("TON: "));
      if (_isTO) {
        DATA_RAW_PRINTLN(F("ON"));
      }else{
        DATA_RAW_PRINTLN(F("OFF"));
      }
/*   
      switch (_isTO) {
        case 1:
          dataPort.println(F("ON unbuffered"));
          break;
        case 2:
          dataPort.println(F("ON buffered"));
          break;
        default:
          dataPort.println(F("OFF"));
      }
*/
    }
    dataPort.println(_isTO);
  }
}


/***** Extract command and pass to handler *****/
int prologixDevice::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(devCmdIndex)/sizeof(*devCmdIndex);

#ifdef DEBUG_PDEV_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nDevice module:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(devCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(devCmdIndex[i].idx));
      }
    }
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(devCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(devCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
int prologixDevice::runCmd(char * cmd, char * params) {

//  uint8_t voidCmdCnt = sizeof(devCmdHidxVoid)/sizeof(*devCmdHidxVoid);
  uint8_t paramCmdCnt = sizeof(devCmdHidxChar)/sizeof(*devCmdHidxChar);

  uint8_t cmdIdxCnt = sizeof(devCmdIndex)/sizeof(*devCmdIndex);

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
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(devCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(devCmdIndex[i].idx));
      break;
    }
  }

  // Handler not found
  if (!idx) {
    // If command not found then return
    #ifdef DEBUG_PDEV_CMD_HANDLER
      DB_PRINT(F("Handler not found!"), "");
    #endif
    return -1;
  }

  // Debug info
  #ifdef DEBUG_PDEV_CMD_HANDLER
    DB_PRINT(F("found handler for: "), cmd);
    DB_PRINT(F("Idx dec: "), idx);
  #endif

  // Handler accepts parameters
    if (idx & PDEV_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (prologixDevice::* mpc)(char *);
//     void *mpcx;
//     mpcx = (void*)pgm_read_ptr(&(comCmdHidxChar[idx].handler));
//     memcpy( &mpc, &mpcx, sizeof(mpc) );
    mpc = devCmdHidxChar[idx].handler;

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_PDEV_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
  /*
  if (idx & PDEV_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (prologixDevice::* mpv)();
//    void *mpvx;
//    mpvx = (void*)pgm_read_ptr(&(comCmdHidxVoid[idx].handler));
//    memcpy( &mpv, &mpvx, sizeof(mpv) );
    mpv = devCmdHidxVoid[(idx & 0x3F)].handler;

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_PDEV_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }
  */

  // No valid command found
  return -1;
}


/*************** ^^^^^^^^^^^^^^^^^ ***************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************************************************/




/***********************************/
/***********************************/
/***** PLACEMENT TO BE DECIDED *****/
/***********************************/
/***********************************/




/***** Show state or enable/disable promiscuous mode *****/
/*
void prom_h(char *params) {
  uint16_t pval;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 1, pval)) return;
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



/***********************************/
/***********************************/
/***** PLACEMENT TO BE DECIDED *****/
/***********************************/
/***********************************/


#endif    // USE_PROLOGIX_DEVICE
