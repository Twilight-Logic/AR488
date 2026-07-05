#include <Arduino.h>
#include "AR488_Config.h"
#include "RATTN_Handler.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||               PROLOGIX DEVICE MODE FUNCTIONS                ||
||                                                             ||
\*=============================================================*/


#ifdef USE_RATTN_HANDLER


rattnHandler::rattnHandler(GPIBbus& bus, inputBuffer& inbuffer, utilityHelper& utilh) : 
                        gpibBus(bus), _inbuf(inbuffer), _util(utilh) {
  _isAddressed = 0;
  _isSpoll = false;
  _parserReset = false;
  _readEoi =- true;

}


/***** Execute a command *****/
int rattnHandler::execCmd(char * cmdexpr) {

  int execStat = -1;

  execStat = runCmd(cmdexpr);
  return execStat;

}


/***** Attention handling routine *****/
/*
 * In device mode is invoked whenever ATN is asserted
 */
void rattnHandler::attnRequired() {

  const uint8_t cmdbuflen = 35;
  uint8_t cmdbytes[cmdbuflen] = {'\0'};
  uint8_t db = 0;
  uint8_t rstat = 0;
  bool eoiDetected = false;
  uint8_t bytecnt = 0;
  uint8_t atnstat = 0;

  // Get ready to listen
  // Set device listner active state (assert NDAC+NRFD (low), set DAV=INPUT_PULLUP)
  gpibBus.setControls(DLAS);

  //----- ATN read loop -----//

  while ( (gpibBus.isAsserted(ATN_PIN)) && (bytecnt<cmdbuflen) ) {
    // Read the next byte from the bus, no EOI detection
    rstat = gpibBus.readByte(&db, false, &eoiDetected);
    if (rstat != HANDSHAKE_COMPLETE) break;
    // Other command
    cmdbytes[bytecnt] = db;
    bytecnt++;
  }

  if (bytecnt > 0) {
    atnstat |= 0x01;
    processATN(cmdbytes, bytecnt, atnstat, rstat);
  }

}


void rattnHandler::processATN(uint8_t cmdbytes[], size_t bytecnt, uint8_t atnstat, uint8_t rstat){

  uint8_t db = 0;
  uint8_t ustat = 0;
  uint8_t gpibcmd = 0;

#ifdef ENABLE_STORAGE
  uint8_t secmode = GPIBNONE;
  uint8_t saddrcmd = 0xFF;
  #ifdef STORAGE_USES_ADDR_0X60
    const uint8_t secbase = 0x60;
  #else
    const uint8_t secbase = 0x61;
  #endif
#endif

  //----- Command process loop -----//

  // Process received command tokens
  for (uint8_t i=0; i<bytecnt; i++) { 

    db = cmdbytes[i];

    // Unlisten
    if (db == 0x3F) {
//      if (device_unl_h()) ustat |= 0x01;
      ustat |= 0x01;
      continue;
    }

    // Untalk
    if (db == 0x5F) {
//      if (device_unt_h()) ustat |= 0x02;
      ustat |= 0x02;
      continue;
    }

    // Primary listen address
    if ( (db>=0x20) && (db<0x40) ) {
      if (gpibBus.cfg.paddr == (db ^ 0x20)) {
        _isAddressed = 0x20;
        atnstat |= 0x02;
        continue;
      }else{
        continue;
      }
    }

    // Primary talk address
    if ( (db>=0x40) && (db<0x60) ){
      if (gpibBus.cfg.paddr == (db ^ 0x40)) {
        _isAddressed = 0x40;
        atnstat |= 0x04;
        continue;
      }else{
        atn_unt_h();
        continue;
      }
    }

#ifdef ENABLE_STORAGE
    // Secondary addresses
    if (db>=secbase && db<0x80) {
      if (_isAddressed) {
        secmode = storage.getSecCmdMode(db);
        if (secmode){
          saddrcmd = db;
          atnstat |= 0x10;
        }else if (db == 0x60){
          saddrcmd = 0x60;
          atnstat |= 0x60;
        }
      }
      break;  // In the case of multiple secondary addresses, break out at the first one
    }
#endif

    // GPIB command bytes
    if (db < 0x31) {
      gpibcmd = db;
      atnstat |= 0x08;
    }

  }   // End for

  // Respond to GPIB command
  if (gpibcmd) {
    execGpibCmd(gpibcmd);
    gpibcmd = 0;
    atnstat |= 0x20;
    return;
  }

  // if we are addressed
  if (_isAddressed) {

#ifdef ENABLE_STORAGE
    if (secmode) {
      // Address has a listen or talk mode
      if ( (_isAddressed & 0x20) && (secmode & GPIBLISTEN) ) { // Listen
        gpibBus.setControls(DLAS);
        atnstat |= 0x02;
      }
      if ( (_isAddressed & 0x40) && (secmode & GPIBTALK) ) { // Talk
        gpibBus.setControls(DTAS);
        atnstat |= 0x04;
      }
      // Execute secondary address command
      storage.storeExecCmd(saddrcmd);
      saddrcmd = 0xFF;
      atnstat |= 0x40;
    }else{
      // Secondary address zero detected - perform GPIB primary command actions
      if (isAddressed & 0x20) {
        gpibBus.setControls(DLAS);  // Listen
        device_listen_h();
      }
//      if (isAddressed & 0x40) {
//        gpibBus.setControls(DTAS);  // Talk
//        device_talk_h(pBuf, PBSIZE);
//      }
//      gpibBus.setControls(DINI);      
    }
#else
    // Perform GPIB primary command actions
    if (_isAddressed & 0x20) {
//      gpibBus.setControls(DLAS);  // Listen
      atn_listen_h();
    }
    if (_isAddressed & 0x40) {
//      gpibBus.setControls(DTAS);  // Talk
      atn_talk_h();
//      gpibBus.setControls(DLAS);  // Talk done - listening again
    }
//      gpibBus.setControls(DINI);
#endif

    if (ustat & 0x01) atn_unl_h();
    if (ustat & 0x02) atn_unt_h();

  } // End isAddressed


#ifdef DEBUG_RATTN_MODULE
  if (bytecnt) {
    showATNStatus(atnstat, ustat, rstat, cmdbytes, bytecnt);
    DB_PRINT(F("END processATN.\n\n"),"");
  }
#endif


}



#ifdef DEBUG_RATTN_MODULE
void rattnHandler::showATNStatus(uint8_t atnstat, uint8_t ustat, uint8_t rstat, uint8_t atnbytes[], size_t bcnt) {

  DB_PRINT(F("Read status: "), rstat);

  if (ustat & 0x01) DB_PRINT(F("unlistened."),"");
  if (ustat & 0x02) DB_PRINT(F("untalked."),"");

  if (atnstat & 0x01) DB_PRINT(F("ATN read loop completed."),"");
  if (atnstat & 0x02) DB_PRINT(F("addressed to LISTEN."),"");
  if (atnstat & 0x04) DB_PRINT(F("addressed to TALK."),"");
  if (atnstat & 0x08) DB_PRINT(F("primary command received."),"");
  if (atnstat & 0x10) DB_PRINT(F("secondary command received."),"");
  if (atnstat & 0x20) DB_PRINT(F("primary command done."),"");
  if (atnstat & 0x40) DB_PRINT(F("secondary command done."),"");
  if (atnstat & 0x80) DB_PRINT(F("data transfer done."),"");

  DB_HEXA_PRINT(F("ATN bytes received: "), atnbytes, bcnt);
  DB_PRINT(bcnt,F(" ATN bytes read."));
//  DB_PRINT(F("Error status: "),rstat);

  DB_PRINT(F("END attnReceived.\n\n"),"");

}
#endif


/***** Execute GPIB command *****/
void rattnHandler::execGpibCmd(uint8_t gpibcmd){

  // Respond to GPIB command
  switch (gpibcmd) {
    case GC_SPE:
        // Serial Poll enable request
#ifdef DEBUG_RATTN_MODULE
        DB_PRINT(F("Received serial poll enable."), "");
#endif
        atn_spe_h();
        break;
    case GC_SPD:
        // Serial poll disable request
#ifdef DEBUG_RATTN_MODULE
        DB_PRINT(F("Received serial poll disable."), "");
#endif
        atn_spd_h();
        break;
/*         
    case GC_UNL:
        // Unlisten
        device_unl_h();
        break;
    case GC_UNT:
        // Untalk
        device_unt_h();
        break;
*/
    case GC_SDC:
        // Device clear (reset)
        atn_sdc_h();
        break;
#ifdef PIN_REMOTE
    case GC_LLO:
        // Remote lockout mode
        atn_llo_h();
        break;
    case GC_GTL:
        // Local mode
        atn_gtl_h();
        break;
#endif
  } // End switch
}


/***** Wait for desired ATN pin state *****/

//NOT NEEDED BECAUSE BY INFERENCE WE HAVE LEFT THE IF (isAsserted(ATN_PIN)) LOOP
/*
void rattnHandler::waitForATN() {
  const unsigned long timeout = millis() + gpibBus.cfg.rtmo;
  while (getGpibPinState(ATN_PIN) == LOW) {
    if (millis() > timeout) break;    // timeout to prevent hung state
    delayMicroseconds(20);
  }
}
*/

/***** Device is addressed to listen - so listen *****/
void rattnHandler::atn_listen_h(){
//  bool atnFree = !gpibBus.isAsserted(ATN_PIN);
//  if (!atnFree) atnFree = _util.waitForPinState(ATN_PIN, 1, gpibBus.cfg.rtmo);
//  if (atnFree) {
    gpibBus.setControls(DLAS);
    gpibBus.receiveData(dataPort, true, false, 0);
//  }
#ifdef DEBUG_RATTN_MODULE
  DB_PRINT(F("Listen done."), "");
#endif
}


/***** Device is addressed to listen - so listen *****/
void rattnHandler::atn_talk_h(){
  DB_PRINT(F("Talk sending: "), _inbuf.data());
//  bool atnFree = !gpibBus.isAsserted(ATN_PIN);
//  if (!atnFree) waitForATN();
//  if (!atnFree) atnFree = _util.waitForPinState(ATN_PIN, 1, gpibBus.cfg.rtmo);
//  if (atnFree && _inbuf.count()) {
//  waitForATN();   // Wait for ATN to go high (unasserted)
  const char * buf = _inbuf.data();
  const size_t cnt = _inbuf.count();
  gpibBus.setControls(DTAS);    // Talk
//  if (atnFree && cnt) {
//  Serial.print(F("Buf: "));
//  Serial.println((unsigned long)_inbuf.data());
//  Serial.print(F("Cnt: "));
  if (cnt) {  
    Serial.println(cnt);
//    delay(50);
    gpibBus.sendData(buf, cnt, true);
//    _inbuf.flush();   // Clear and reset the input buffer
    _parserReset = true;
  }else{
    gpibBus.sendData("\0",1,true);
  }
//  gpibBus.setControls(DIDS);    // Listen

#ifdef DEBUG_RATTN_MODULE
  DB_PRINT(F("Talk done."), "");
#endif
}


/***** Selected Device Clear *****/
void rattnHandler::atn_sdc_h() {
  // Device clear
  #ifdef DEBUG_RATTN_MODULE
    DB_PRINT(F("SDC requested..."), "");
  #endif
  _parserReset = true;
  gpibBus.cfg.stat = 0;
  gpibBus.clearSignal(SRQ_BIT);
  #ifdef DEBUG_RATTN_MODULE
    DB_PRINT(F("Done."), "");
  #endif
}


/***** Serial Poll Disable Request *****/
void rattnHandler::atn_spd_h() {
#ifdef DEBUG_RATTN_MODULE
  DB_PRINT(F("<- serial poll request ended."),"");
#endif
  gpibBus.setControls(DIDS);
}


/***** Serial Poll Enable Request *****/
void rattnHandler::atn_spe_h() {
#ifdef DEBUG_RATTN_MODULE
  DB_PRINT(F("Serial poll request received ->"), "");
#endif
  gpibBus.sendStatus();
#ifdef DEBUG_RATTN_MODULE
  DB_PRINT(F("Status sent."), "");
#endif
  // Check if SRQ bit is set and clear it
  if (gpibBus.cfg.stat & 0x40) {
    gpibBus.setStatus(gpibBus.cfg.stat & ~0x40);
#ifdef DEBUG_RATTN_MODULE
    DB_PRINT(F("SRQ bit cleared."), "");
#endif
  }
}


/***** Unlisten *****/
bool rattnHandler::atn_unl_h() {
  // Stop receiving and go to idle
/***** NEEDS TO BE FIXED *****/
//  readWithEoi = false;
/***** NEEDS TO BE FIXED *****/
  // Clear addressed state flag and set controls to idle
  if (gpibBus.isDeviceAddressedToListen()) {
    gpibBus.setControls(DIDS);
    return true;
  }
  return false;
}


/***** Untalk *****/
bool rattnHandler::atn_unt_h(){
  // Clear addressed state flag and set controls to listen
  if (gpibBus.isDeviceAddressedToTalk()) {
    gpibBus.setControls(DIDS);
    gpibBus.clearDataBus();
    return true;
  }
  return false;
}



/***** Arrays containing index of accepted ++ commands *****/
/*
* Format: token, function_ptr
*/

const rattnHandler::ratCmdRecVoid rattnHandler::ratCmdHidxVoid [] = {
};

const rattnHandler::ratCmdRecChar rattnHandler::ratCmdHidxChar [] = {
};


/***** Extract command pointer and pass to handler *****/
int rattnHandler::runCmd(char * cmd) {

  char *token;  // Pointer to command token

  uint8_t voidCmdCnt = sizeof(ratCmdHidxVoid)/sizeof(*ratCmdHidxVoid);
  uint8_t paramCmdCnt = sizeof(ratCmdHidxChar)/sizeof(*ratCmdHidxChar);

#ifdef DEBUG_RATN_CMD_HANDLER
  DB_PRINT(F("command buffer: "), cmd);
#endif

  // If non-printable character on blank line then return immediately
  if (cmd[0] < 0x20) return 0;

  // Get the first token
  token = strtok(cmd, " \t");

#ifdef DEBUG_RATN_CMD_HANDLER
  DB_PRINT(F("process token: "), token);
#endif

  int idx = 0;

  if (token) {
    uint8_t tlen = 0;
    for (uint8_t i=0; i<voidCmdCnt; i++) {
      tlen = strlen(ratCmdHidxVoid[i].token);
      if ( (strncasecmp(ratCmdHidxVoid[i].token, token, tlen) == 0 ) ) {
        idx = ( i | RATN_FUNC_TYPE_VOID );
        break;
      }
    }
    if (!idx) {
      for (uint8_t i=0; i<paramCmdCnt; i++) {
        tlen = strlen(ratCmdHidxChar[i].token);
        if ( (strncasecmp(ratCmdHidxChar[i].token, token, tlen) == 0 ) ) {
          idx = ( i | RATN_FUNC_TYPE_PARAM );
          break;
        }
      } 
    }

    // If command not found then return
    if (!idx) {
      // Re-instate space removed by strtok
      cmd[strlen(token)] = 0x20;
      #ifdef DEBUG_RATN_CMD_HANDLER
        DB_PRINT(F("Handler not found!"), "");
      #endif
      return -1;
    }

    // Otherwise run the command
    #ifdef DEBUG_RATN_CMD_HANDLER
      DB_PRINT(F("found handler for: "), token);
      DB_RAW_PRINT(F("Idx: "));
      DB_HEX_PRINT(idx);
      DB_RAW_PRINTLN();
    #endif

    if (idx & 0x80) {
      void (rattnHandler::* mpc)(char *);
      char * params = strtok(NULL, "\0");
      mpc = ratCmdHidxChar[(idx & 0x3F)].handler;
      #ifdef DEBUG_RATN_CMD_HANDLER
        DB_PRINT(F("calling: "), token);
        DB_PRINT(F("params:  "), params);
        DB_HEXB_PRINT(F("params hex: "), params, strlen(params));
      #endif
      // Call handler with parameters specified
      (*this.*mpc)(params);

      #ifdef DEBUG_RATN_CMD_HANDLER
        DB_PRINT(F("handler done."),"");
      #endif
      return 0;
    }

    if (idx & 0x40){
      void (rattnHandler::* mpv)();
      mpv = ratCmdHidxVoid[(idx & 0x3F)].handler;
      #ifdef DEBUG_RATN_CMD_HANDLER
        DB_PRINT(F("calling: "), token);
      #endif
      // Call handler with parameters specified
      (*this.*mpv)();
      #ifdef DEBUG_RATN_CMD_HANDLER
        DB_PRINT(F("handler done."),"");
      #endif
      return 0;
    }

  }

  // No valid command found
  return -1; 
}



#ifdef REMOTE_SIGNAL_PIN
/***** Disable remote mode *****/
void rattnHandler::atn_gtl_h(){
  digitalWrite(REMOTE_SIGNAL_PIN, LOW);
}


/***** Enabled remote mode *****/
void rattnHandler::atn_llo_h(){
  digitalWrite(REMOTE_SIGNAL_PIN, HIGH);
}
#endif


bool rattnHandler::resetParser(){
  bool result = _parserReset;
  _parserReset = false;
  return result;
}


bool rattnHandler::readWithEoi(){
  return _readEoi;
}


#endif  // #endif  // USE_RATTN_HANDLER

