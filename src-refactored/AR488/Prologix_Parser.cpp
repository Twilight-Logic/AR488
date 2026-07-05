#include <Arduino.h>
#include "AR488_Config.h"
#include "Prologix_Parser.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                       PROLOGIX PARSER                       ||
||                                                             ||
\*=============================================================*/




/*********************************/
/***** PROLOGIX PARSER CLASS *****/
/***** vvvvvvvvvvvvvvvvvvvvv *****/


prologixParser::prologixParser(inputBuffer& inBuf, GPIBbus &bus, utilityHelper& utils, AR488modulesList& modules) : _inBuf(inBuf), _gpibBus(bus), _utils(utils), _modules(modules) {
  _state = 0;
  _isEsc = false;
  _isPlusEsc = false;
  _idnQuery = false;
//  _isRO = false;
}


void prologixParser::init(){

  // Initialise parser
  #ifdef DEBUG_PARSER
    DB_PRINT(F("Initialising parser..."),"");
  #endif 
  reset();
  #ifdef DEBUG_PARSER
    DB_PRINT(F("Done."),"");
  #endif

}


/***** Add character to the buffer and parse *****/
void prologixParser::parseInput(char c) {

  // Read until buffer full
  if (_inBuf.count() < PBUFSIZE) {
    if (isVerb && c!=LF) dataPort.print(c);  // Humans like to see what they are typing...
    // Actions on specific characters
    switch (c) {
      // Carriage return or newline? Then process the line
      case CR:
      case LF:
        // If escaped add char 0x10 or 0x13 to buffer and clear Escape flag
        if (_isEsc) {
          _inBuf.add(c);
          _isEsc = false;
        } else {
          // Carriage return on blank line?
          // Note: for data CR and LF will always be escaped
          if (_inBuf.count() == 0) {
            _inBuf.flush();
            if (isVerb) {
              DATA_RAW_PRINTLN();
              _utils.showPrompt();
            }
            _state = 0;
            return;
          } else {
#ifdef DEBUG_PARSER
            DB_PRINT(F("Received: "), _inBuf.data());
#endif
            // Buffer starts with ++ and contains at least 3 characters - command?
            if (_inBuf.count()>2 && _inBuf.hasCmd() && !_isPlusEsc) {
              // Exclamation mark (break read loop command)
              if (_inBuf.getChar(2)==0x21) {
                _state = 3;
                _inBuf.flush();
              // Otherwise flag command received and ready to process 
              }else{
                _state = 1;
              }
            // Buffer has at least 1 character = instrument data to send to gpib bus
            }else if (_inBuf.count() > 0) {
              _state = 2;
            }
            _isPlusEsc = false;
#ifdef DEBUG_PARSER
            DB_PRINT(F("State: "), _state);
#endif
          }
        }
        break;
      case ESC:
        // Handle the escape character
        if (_isEsc) {
          // Add character 0x27 to buffer and clear Escape flag
          _inBuf.add(c);
          _isEsc = false;
        } else {
          // Flag that we have seen an Escape character
          _isEsc  = true;
        }
        break;
      case PLUS:
        if (_isEsc) {
          _isEsc = false;
          if (_inBuf.count() < 2) _isPlusEsc = true;
        }
        _inBuf.add(c);
        break;
      // Something else?
      default: // any char other than defined above
        _inBuf.add(c);
        _isEsc = false;
    }
  }
  if (_inBuf.count() >= PBUFSIZE) {
    if (_inBuf.hasCmd() && !_state) {  // Command without terminator and buffer full
      if (isVerb) {
        dataPort.println(F("ERROR - overflow!"));
      }
      _inBuf.flush();
    }else{  // Buffer contains data and is full, so process the buffer (send data via GPIB)
      _state = 2;
    }
  }
}


/***** Command execution handler *****/
/*
 * If a command is detected, it will be passed to each module in turn until
 * a module confirms it has executed the command by returning the approprate
 * value. Return values are:
 * -1 : Command not found  
 *  0 : Command executed normally
 * >0 : Command executed, error ocurred
*/
void prologixParser::findExecCmd(char * cmdexpr, uint8_t cxlen){
  int execStatus = -1;

  const uint8_t cmdlen = cxlen - 2;

  if (!cmdlen) return;

  if (isVerb) dataPort.println();

  // Remove '++' by shifting everything two bytes left
  // Replace last two bytes with a NULL character
  for (int i = 0; i < cmdlen; i++) {
    cmdexpr[i] = cmdexpr[i + 2];
  }
  cmdexpr[cmdlen] = '\0';
  cmdexpr[cmdlen+1] = '\0';

#ifdef DEBUG_CMD_HANDLER
  DB_PRINT(F("command received (ASC): "), cmdexpr);
  DB_HEXB_PRINT(F("command received (HEX): "), cmdexpr, cmdlen);
#endif

  // Extract command and parameters
  char * cmd = strtok(cmdexpr, " \t");
  char * params = strtok(NULL, "\0");

  // If its a help command then print help
  if (strcasecmp(cmd, "help") == 0) {
    if (strlen(params)) {
      printHelp(params, true);
    }else{
      printHelp(params, false);
    }
    return;
  }

  // Else pass command to module command processors 
#ifdef USE_PROLOGIX_COMMON
  execStatus = _modules.pcommon->execCmd(cmd, params);
#endif
#ifdef USE_PROLOGIX_CONTROLLER
  if (execStatus<0) execStatus = _modules.pcontroller->execCmd(cmd, params);
#endif
#ifdef USE_PROLOGIX_DEVICE
  if (execStatus<0) execStatus = _modules.pdevice->execCmd(cmd, params);
#endif
/*
#ifdef USE_RATTN_HANDLER
  if (execStatus<0) execStatus = _modules.rattn->execCmd(cmd, params);
#endif
*/
#ifdef USE_EEPROM_HANDLER
  if (execStatus<0) execStatus = _modules.eeprom->execCmd(cmd, params);
#endif
#ifdef USE_MACRO_HANDLER_PM
  if (execStatus<0) execStatus = _modules.macropm->execCmd(cmd, params);
#endif
#ifdef USE_DIAGNOSTIC_HELPER
  if (execStatus<0) execStatus = _modules.diag->execCmd(cmd, params);
#endif

/*
#ifdef DEBUG_CMD_HANDLER
  DB_PRINT(F("sent to command processor:"), cmdexpr);
#endif
*/
  // Process result
  if (execStatus) _utils.errorMsg(execStatus);

#ifdef DEBUG_CMD_HANDLER
  DB_PRINT(F("result: "), execStatus);
#endif

  if (isVerb) _utils.showPrompt();

}



void prologixParser::printHelp(char * token, bool find) {

  int helpStatus = -1;

#ifdef DEBUG_CMD_HANDLER
  DB_PRINT(F("print help for: "), token);
#endif

  // Pass command to module command processors 
#ifdef USE_PROLOGIX_COMMON
  helpStatus = _modules.pcommon->getHelp(token, find);
#endif
#ifdef USE_PROLOGIX_CONTROLLER
  if (helpStatus<0) helpStatus = _modules.pcontroller->getHelp(token, find);
#endif
#ifdef USE_PROLOGIX_DEVICE
  if (helpStatus<0) helpStatus = _modules.pdevice->getHelp(token, find);
#endif
/*
#ifdef USE_RATTN_HANDLER
  if (helpStatus<0) helpStatus = _modules.rattn->getHelp(token, find);
#endif
*/
#ifdef USE_EEPROM_HANDLER
  if (helpStatus<0) helpStatus = _modules.eeprom->getHelp(token, find);
#endif
#ifdef USE_MACRO_HANDLER_PM
  if (helpStatus<0) helpStatus = _modules.macropm->getHelp(token, find);
#endif
#ifdef USE_DIAGNOSTIC_HELPER
  if (helpStatus<0) helpStatus = _modules.diag->getHelp(token, find);
#endif

  // Process result
//  if (helpStatus) _utils.errorMsg(execStatus);

#ifdef DEBUG_CMD_HANDLER
  DB_PRINT(F("result: "), helpStatus);
#endif

}



/***** Serial event handler *****/
/*
 * Note: the Arduino serial buffer is 64 characters long. Characters are stored in
 * this buffer until prologixParser::parse() is called. prologixParser::parseInput()
 * takes a character at a time and places it into the character parse buffer whereupon
 * it is parsed to determine whether a command or data are present.
 * state=0: waiting: terminator not detected yet
 * state=1: ready: terminator detected, sequence in parse buffer is a ++ command
 * state=2: Ready: terminator detected, sequence in parse buffer is to be sent directly to the instrument
 */ 
void prologixParser::parse() {
  // Parse serial input until we have detected a line terminator
  while ( dataPort.available() && !_state ) {   // Parse while characters available and line is not complete
    parseInput(dataPort.read());
  }
  checkState();
}


//void prologixParser::parseMacro(char *& macro, size_t msize) {
void prologixParser::parseMacro(macroHandlerPM& mhandler) {
  size_t cnt=0;
  size_t msize = mhandler.available();
  char c;

  if (msize) {

    #ifdef DEBUG_PARSER
      DB_PRINT(F("Parsing macro:"),"");
    #endif

    // Parse serial input until we have detected a line terminator
    while ( cnt < msize ) {
      c = mhandler.charAt(cnt);
      #ifdef DEBUG_PARSER
        DB_DATA_WRITE(c); 
      #endif
      parseInput(c);
      cnt++;
      if (_state) {
        checkState();
      }
    }

    mhandler.clearMacro();

    #ifdef DEBUG_PARSER
      DB_PRINT(F("Bytes read: "), msize);
    #endif

  }

}


void prologixParser::checkState() {
  // Buffer contains a ++ command
  if (_state == 1) {
    findExecCmd(_inBuf.data(), _inBuf.count());
    reset();
  }
  // Buffer contains data
  if (_state == 2) {
  //  if (_pdevice.isTonEnabled()) {
  //    _pdevice.tonMode(_inBuf.data(), _inBuf.count());
  //  }else{
    #if defined(USE_PROLOGIX_CONTROLLER)
      // In controller mode we send the data to the instrument
      if (_gpibBus.cfg.cmode == 2) {
        _modules.pcontroller->sendToInstrument(_inBuf.data(), _inBuf.count());
      }
    #endif
    #if defined(USE_PROLOGIX_DEVICE)
      // In device mode we check the device is not in listen only mode before sending data
      if (_gpibBus.cfg.cmode == 1) {
        if ( !_modules.pdevice->isLonEnabled() ) _gpibBus.sendData(_inBuf.data(), _inBuf.count(), _gpibBus.cfg.eoi);
      }
    #endif
    reset();
  }
  // Buffer contains ++! break command
  if (_state == 3) {
    // Stop autoread
    #ifdef USE_PROLOGIX_CONTROLLER
    _modules.pcontroller->autorun(false);
    reset();
    #endif
  }
  #ifdef USE_PROLOGIX_CONTROLLER
  // Autoread command issued
// CONDITION REQUIRED?  
  _modules.pcontroller->autorun(true);
// CONDITION REQUIRED?  
  #endif
}


void prologixParser::reset(){
  _state = 0;
  _isEsc = false;
  _isPlusEsc = false;
  _idnQuery = false;
  _inBuf.flush();
}


uint8_t prologixParser::ready() {
  return _state;
}


size_t prologixParser::count() { 
  return _inBuf.count();
}


char * prologixParser::data() {
  return _inBuf.data();
}


#ifdef USE_RATTN_HANDLER
void prologixParser::setTalkNow(bool talk) {
  _talk = talk;
}
#endif

/***** ^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PROLOGIX PARSER CLASS *****/
/*********************************/
