#include <Arduino.h>
#include "AR488_Config.h"
#include "Prologix_Common.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                 PROLOGIX COMMON FUNCTIONS                   ||
||                                                             ||
\*=============================================================*/



/***** Default version string *****/
#if defined(__AVR__)
  const char fwver[] PROGMEM = FW_VERSION;
#else
  const char fwver[] = FW_VERSION;
#endif


/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


prologixCommon::prologixCommon(GPIBbus& bus, utilityHelper& utilh) :
                        gpibBus(bus), _util(utilh) {

}


void prologixCommon::init() {

}


/***** Execute a command *****/
int prologixCommon::execCmd(char * cmd, char * params) {

  int execStat = -1;

  execStat = runCmd(cmd, params);
  return execStat;

}


/***** Pointer to extended commands help *****/
int prologixCommon::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}


/************** ^^^^^^^^^^^^^^^^ ***************/
/* <----------  PUBLIC FUNCTIONS   ----------> */
/***********************************************/




/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/

const char cmd01[] PROGMEM = "addr";
const char cmd02[] PROGMEM = "auto_int";
const char cmd03[] PROGMEM = "eoi";
const char cmd04[] PROGMEM = "eor";
const char cmd05[] PROGMEM = "eos";
const char cmd06[] PROGMEM = "eot_char";
const char cmd07[] PROGMEM = "eot_enable";
const char cmd08[] PROGMEM = "id";
const char cmd09[] PROGMEM = "mode";
const char cmd10[] PROGMEM = "read_tmo_ms";
const char cmd11[] PROGMEM = "rst";
const char cmd12[] PROGMEM = "ver";
const char cmd13[] PROGMEM = "verbose";

const char cmd01desc[] PROGMEM = "\t\tDisplay/set device address";
const char cmd02desc[] PROGMEM = "\tAutoread interval in milliseconds (when auto = 3)";
const char cmd03desc[] PROGMEM = "\t\tEnable|disable assertion of EOI signal";
const char cmd04desc[] PROGMEM = "\t\tShow or set end of receive character(s)";
const char cmd05desc[] PROGMEM = "\t\tSpecify GPIB termination character";
const char cmd06desc[] PROGMEM = "\tSet character to append to USB output when EOT enabled";
const char cmd07desc[] PROGMEM = "\tEnable/Disable appending user specified character to USB output on EOI detection";
const char cmd08desc[] PROGMEM = "\t\tShow/set interface ID information: id (shows all); id name; id serial; id verstr";
const char cmd09desc[] PROGMEM = "\t\tSet the interface mode (1=controller; 0=device)";
const char cmd10desc[] PROGMEM = "\tRead timeout specified between 1 - 3000 milliseconds";
const char cmd11desc[] PROGMEM = "\t\tReset the controller";
const char cmd12desc[] PROGMEM = "\t\tDisplay firmware version";
const char cmd13desc[] PROGMEM = "\tToggle verbose (human readable) mode";

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
const prologixCommon::comCmdTxtIdx prologixCommon::comCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | PCOM_FUNC_TYPE_PARAM), cmd01desc },
  { cmd02, (0x01 | PCOM_FUNC_TYPE_PARAM), cmd02desc },
  { cmd03, (0x02 | PCOM_FUNC_TYPE_PARAM), cmd03desc },
  { cmd04, (0x03 | PCOM_FUNC_TYPE_PARAM), cmd04desc },
  { cmd05, (0x04 | PCOM_FUNC_TYPE_PARAM), cmd05desc },
  { cmd06, (0x05 | PCOM_FUNC_TYPE_PARAM), cmd06desc },
  { cmd07, (0x06 | PCOM_FUNC_TYPE_PARAM), cmd07desc },
  { cmd08, (0x07 | PCOM_FUNC_TYPE_PARAM), cmd08desc },
  { cmd09, (0x08 | PCOM_FUNC_TYPE_PARAM), cmd09desc },
  { cmd10, (0x09 | PCOM_FUNC_TYPE_PARAM), cmd10desc },
  { cmd11, (0x00 | PCOM_FUNC_TYPE_VOID),  cmd11desc },
  { cmd12, (0x0A | PCOM_FUNC_TYPE_PARAM), cmd12desc },
  { cmd13, (0x01 | PCOM_FUNC_TYPE_VOID),  cmd13desc }
};


/***** Array containing index of accepted ++ void commands *****/
const prologixCommon::comCmdRecVoid prologixCommon::comCmdHidxVoid [] = {
  { &prologixCommon::rst_h  },
  { &prologixCommon::verb_h }
};


/***** Array containing index of accepted ++ commands with parameters*****/
const prologixCommon::comCmdRecChar prologixCommon::comCmdHidxChar [] = {
  { &prologixCommon::addr_h     },
  { &prologixCommon::aint_h     },
  { &prologixCommon::eoi_h      },
  { &prologixCommon::eor_h      },
  { &prologixCommon::eos_h      },
  { &prologixCommon::eot_char_h },
  { &prologixCommon::eot_en_h   },
  { &prologixCommon::id_h       },
  { &prologixCommon::cmode_h    },
  { &prologixCommon::rtmo_h     },
  { &prologixCommon::ver_h      }
};


/***** Extract command and pass to handler *****/
int prologixCommon::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(comCmdIndex)/sizeof(*comCmdIndex);

#ifdef DEBUG_PCOM_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nCommon commands module:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(comCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(comCmdIndex[i].idx));
      }
    }
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(comCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(comCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
//int prologixCommon::runCmd(char * cmd) {
int prologixCommon::runCmd(char * cmd, char * params) {

  uint8_t voidCmdCnt = sizeof(comCmdHidxVoid)/sizeof(*comCmdHidxVoid);
  uint8_t paramCmdCnt = sizeof(comCmdHidxChar)/sizeof(*comCmdHidxChar);

  uint8_t cmdIdxCnt = sizeof(comCmdIndex)/sizeof(*comCmdIndex);

#ifdef DEBUG_PCOM_CMD_HANDLER
  DB_PRINT(F("command buffer: "), cmd);
#endif

  // If parameter is empty then return immediately
  if (!cmd) return -1;
  // If non-printable character on blank line then return immediately
  if (cmd[0] < 0x20) return 0;

  int idx = 0;

  // Find the handler
  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(comCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(comCmdIndex[i].idx));
      break;
    }
  }

  // Handler not found
  if (!idx) {
    // If command not found then return
    #ifdef DEBUG_PCOM_CMD_HANDLER
      DB_PRINT(F("Handler not found!"), "");
    #endif
    return -1;
  }

  // Debug info
  #ifdef DEBUG_PCOM_CMD_HANDLER
    DB_PRINT(F("found handler for: "), cmd);
    DB_PRINT(F("Idx dec: "), idx);
  #endif

  // Run the handler

  // Handler accepts parameters
    if (idx & PCOM_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (prologixCommon::* mpc)(char *);
//     void *mpcx;
//     mpcx = (void*)pgm_read_ptr(&(comCmdHidxChar[idx].handler));
//     memcpy( &mpc, &mpcx, sizeof(mpc) );
    mpc = comCmdHidxChar[idx].handler;

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_PCOM_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
  if (idx & PCOM_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (prologixCommon::* mpv)();
//    void *mpvx;
//    mpvx = (void*)pgm_read_ptr(&(comCmdHidxVoid[idx].handler));
//    memcpy( &mpv, &mpvx, sizeof(mpv) );
    mpv = comCmdHidxVoid[(idx & 0x3F)].handler;

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_PCOM_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // No valid command found
  return -1;
}


/*****************************/
/***** UTILITY FUNCTIONS *****/
/***** vvvvvvvvvvvvvvvvv *****/


void prologixCommon::printFWver(){
#if defined(__AVR__)
  dataPort.println((__FlashStringHelper*)fwver);
#else
  dataPort.println(fwver);
#endif
}


void prologixCommon::printId(uint8_t idx, bool header){

  if (idx & 0x01) {
    if (header) dataPort.print(F("FWver:  "));
    printFWver();
  }
  if (idx & 0x02) {
    if (header) dataPort.print(F("VerStr: "));
    if (gpibBus.cfg.vstr[0]) {
      dataPort.println(gpibBus.cfg.vstr);
    } else {
      printFWver();
    }
  }
  if (idx & 0x04) {
    if (header) dataPort.print(F("Name:   "));
    dataPort.println(gpibBus.cfg.sname);
  }
  if (idx & 0x08) {
    char serialStr[10];
    unsigned long int serialnum = gpibBus.cfg.serial;
    memset(serialStr, '\0', 10);
    snprintf(serialStr, 10, "%09lu", serialnum);  // Max str length = 10-1 i.e 9 digits + null terminator
    if (header) dataPort.print(F("Serial: "));
    dataPort.println(serialStr);
  }

}


/***** ^^^^^^^^^^^^^^^^^ *****/
/***** UTILITY FUNCTIONS *****/
/*****************************/





/************************************************/
/***** PROLOGIX COMPATIBLE COMMAND HANDLERS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/



/***** Show or change device address *****/
void prologixCommon::addr_h(char *params) {
  char *param;
  uint16_t val;
  uint8_t saddr;

  if (params) {
    // Primary address
    param = strtok(params, ", \t");
    if (!_util.isNumber(param)){
      _util.errorMsg(2);
      return;
    }
    if (_util.notInRange(param, 0, 30, val)) return;
    if (val == gpibBus.cfg.caddr) {
      _util.errorMsg(2);
      if (isVerb) dataPort.println(F("Cannot address the controller!"));
      return;
    }
    gpibBus.cfg.paddr = val;

    // Secondary address
    gpibBus.cfg.saddr = 0xFF; // Default
    param = strtok(NULL, ", \t");
    if (param != NULL) {
      if (_util.isNumber(param)){
        saddr = atoi(param);
      }else{
        _util.errorMsg(2);
        return;
      }
      if (saddr<31) saddr += 0x60;
      if (saddr<96 || saddr>126) {
        _util.errorMsg(2);
        return;
      }
      gpibBus.cfg.saddr = saddr;
    }
/*
    if (isVerb) {
      dataPort.print(F("PRI address: "));
      dataPort.println(gpibBus.cfg.paddr);
      dataPort.print(F("SEC address: "));
      if (gpibBus.cfg.saddr == 0xFF) {
        dataPort.println(F("unset"));
      }else{
        dataPort.println(gpibBus.cfg.saddr);
      }
    }
*/
  } else {
    dataPort.print(gpibBus.cfg.paddr);
    if (gpibBus.cfg.saddr != 0xFF) {
      dataPort.print(',');
      dataPort.print(gpibBus.cfg.saddr);
    }
    dataPort.println();
  }
}


/***** Show or set interface to controller/device mode *****/
void prologixCommon::cmode_h(char *params) {
  uint16_t val;
  if (params) {
    if (_util.notInRange(params, 0, 1, val)) return;
    switch (val) {
      case 0:
        #ifdef USE_PROLOGIX_DEVICE
          gpibBus.startDeviceMode();
        #else
          DATA_RAW_PRINTLN(F("Not available."));
        #endif
        break;
      case 1:
        #ifdef USE_PROLOGIX_DEVICE
          gpibBus.startControllerMode();
        #else
          DATA_RAW_PRINTLN(F("Not available."));
        #endif
        break;
    }

    if (isVerb) {
      dataPort.print(F("Interface mode set to: "));
      dataPort.println(val ? "CONTROLLER" : "DEVICE");
    }

  } else {
    dataPort.println(gpibBus.isController());
  }
}


/***** Show or set EOI assertion on/off *****/
void prologixCommon::eoi_h(char *params) {
  uint16_t val;
  if (params) {
    if (_util.notInRange(params, 0, 1, val)) return;
    gpibBus.cfg.eoi = val ? true : false;
/*
    if (isVerb) {
      dataPort.print(F("Set EOI assertion: "));
      dataPort.println(val ? "ON" : "OFF");
    };
*/    
  } else {
    dataPort.println(gpibBus.cfg.eoi);
  }
}


/***** Show or set end of send character *****/
void prologixCommon::eos_h(char *params) {
  uint16_t val;
  if (params) {
    if (_util.notInRange(params, 0, 3, val)) return;
    gpibBus.cfg.eos = (uint8_t)val;
/*
    if (isVerb) {
      dataPort.print(F("Set EOS to: "));
      dataPort.println(val);
    };
*/
  } else {
    dataPort.println(gpibBus.cfg.eos);
  }
}


/***** Show or set end of transmission character *****/
void prologixCommon::eot_char_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 255, val)) return;
    gpibBus.cfg.eot_ch = (uint8_t)val;
/*
    if (isVerb) {
      dataPort.print(F("EOT set to ASCII character: "));
      dataPort.println(val);
    };
*/
  } else {
    dataPort.println(gpibBus.cfg.eot_ch, DEC);
  }
}


/***** Show or enable/disable sending of end of transmission character *****/
void prologixCommon::eot_en_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 1, val)) return;
    gpibBus.cfg.eot_en = val ? true : false;
/*
    if (isVerb) {
      dataPort.print(F("Appending of EOT character: "));
      dataPort.println(val ? "ON" : "OFF");
    }
*/
  } else {
    dataPort.println(gpibBus.cfg.eot_en);
  }
}


/***** Show or set read timout *****/
void prologixCommon::rtmo_h(char * params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 1, 32000, val)) return;
    gpibBus.cfg.rtmo = val;
/*
    if (isVerb) {
      dataPort.print(F("Set <read_tmo_ms> to: "));
      dataPort.print(val);
      dataPort.println(F(" milliseconds"));
    }
*/
  } else {
    dataPort.println(gpibBus.cfg.rtmo);
  }
}


//void prologixCommon::rst_h(char * params){
void prologixCommon::rst_h(){
  _util.mcuReset();
}


/***** Display the controller version string *****/
void prologixCommon::ver_h(char *params) {
  // If "real" requested
  if (params && strcasecmp(params, "real") == 0) {
    printId(0x01, false);
  // Otherwise depends on whether we have a custom string set
  } else {
    printId(0x02, false);
  }
}


/***** Enable verbose mode 0=OFF; 1=ON *****/
void prologixCommon::verb_h() {
  isVerb = !isVerb;
  dataPort.print("Verbose: ");
  dataPort.println(isVerb ? "ON" : "OFF");
}


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PROLOGIX COMPATIBLE COMMAND HANDLERS *****/
/************************************************/



/*************************************/
/***** EXTENDED COMMAND HANDLERS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/


/***** Show or set auto-read interval *****/
void prologixCommon::aint_h(char * params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 1, 32000, val)) return;
    gpibBus.cfg.aint = val;
/*
    if (isVerb) {
      dataPort.print(F("Set <auto_int> to: "));
      dataPort.print(val);
      dataPort.println(F(" milliseconds"));
    }
*/    
  } else {
    dataPort.println(gpibBus.cfg.aint);
  }
}


/***** Show or set end of receive character(s) *****/
void prologixCommon::eor_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (_util.notInRange(params, 0, 7, val)) return;
    gpibBus.cfg.eor = (uint8_t)val;
/*    
    if (isVerb) {
      dataPort.print(F("Set EOR to: "));
      dataPort.println(val);
    };
*/
  } else {
    if (gpibBus.cfg.eor>7) gpibBus.cfg.eor = 0;  // Needed to reset FF read from EEPROM after FW upgrade
    dataPort.println(gpibBus.cfg.eor);
  }
}


/***** Set device ID *****/
/*
 * Sets the device ID parameters including:
 * ++id verstr - version string (same as ++setvstr)
 * ++id name   - short name of device (e.g. HP3478A) up to 15 characters
 * ++id serial - serial number up to 9 digits long
 */
void prologixCommon::id_h(char *params) {
  uint8_t dlen = 0;
  char * keyword; // Pointer to keyword following ++id
  char * datastr; // Pointer to supplied data (remaining characters in buffer)
//  char serialStr[10];

#ifdef DEBUG_IDFUNC
  DB_PRINT(F("Params: "), params);
#endif

  if (params != NULL) {
    keyword = strtok(params, " \t");
    datastr = keyword + strlen(keyword) + 1;
    dlen = strlen(datastr);
    if (dlen) {
      if (strncasecmp(keyword, "verstr", 6)==0) {
#ifdef DEBUG_IDFUNC
        DB_PRINT(F("Keyword: "), keyword);
        DB_PRINT(F("DataStr: "), datastr);
#endif
        if (dlen>0 && dlen<48) {
#ifdef DEBUG_IDFUNC
        DB_PRINT(F("Length OK"),"");
#endif
          memset(gpibBus.cfg.vstr, '\0', 48);
          strncpy(gpibBus.cfg.vstr, datastr, dlen);
          if (isVerb) {
            dataPort.print(F("VerStr: "));
            dataPort.println(gpibBus.cfg.vstr);
          }
        }else{
          if (isVerb) dataPort.println(F("Length > 48 characters!"));
          _util.errorMsg(2);
        }
        return;
      }
      if (strncasecmp(keyword, "name", 4)==0) {
        if (dlen>0 && dlen<16) {
          memset(gpibBus.cfg.sname, '\0', 16);
          strncpy(gpibBus.cfg.sname, datastr, dlen);
        }else{
          if (isVerb) dataPort.println(F("Length > 15 characters!"));
          _util.errorMsg(2);
        }
        return;
      }
      if (strncasecmp(keyword, "serial", 6)==0) {
        if (dlen < 10) {
          gpibBus.cfg.serial = atol(datastr);
        }else{
          if (isVerb) dataPort.println(F("Length > 9 characters!"));
          _util.errorMsg(2);
        }
        return;
      }
    }else{
      if (strncasecmp(keyword, "verstr", 6)==0) {
          printId(0x02, false);
        return;
      }
      if (strncasecmp(keyword, "fwver", 6)==0) {
          printId(0x01, false);
        return;
      }
      if (strncasecmp(keyword, "name", 4)==0) {
        printId(0x04, false);
        return;      
      } void addr_h(char *params);
      if (strncasecmp(keyword, "serial", 6)==0) {
        printId(0x08, false);
        return;    
      }
    }
  }else{
    printId(0xFF, true);
  }
//  _util.errorMsg(0);
#ifdef DEBUG_IDFUNC
    DB_PRINT(F("done."),"");
#endif
}



/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** EXTENDED COMMAND HANDLERS *****/
/*************************************/


/*************** ^^^^^^^^^^^^^^^^^ ***************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************************************************/

