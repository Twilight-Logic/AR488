#include "Macro_Handler_PM.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                        MACRO HELPER                         ||
||                                                             ||
\*=============================================================*/


#ifdef USE_MACRO_HANDLER_PM


/************************************************/
/* <----------   PUBLIC FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvv ***************/


static const char macCmdHelpExtended[] PROGMEM = {
  "macro:\t\tRun a macro (if macro support is compiled)\n"
};


macroHandlerPM::macroHandlerPM(inputBuffer& inBuf, utilityHelper& utils) : _inBuf(inBuf), _util(utils) {
  _currentMacro = NULL;
  _msize = 0;
}


/***** Execute a command *****/
int macroHandlerPM::execCmd(char * cmd, char * params) {

  int execStat = -1;

  execStat = runCmd(cmd, params);
  return execStat;

}


/***** Pointer to extended commands help *****/
int macroHandlerPM::getHelp(char * token, bool find){
  return getHelpInfo(token, find);
}


size_t macroHandlerPM::available() const {
  return _msize;
}


char macroHandlerPM::charAt(size_t idx) const {
  if (_currentMacro != nullptr){
    if (idx < _msize) return (char)pgm_read_byte_near(_currentMacro + idx);
  }
  return '\0';
}


void macroHandlerPM::clearMacro() {
  _currentMacro = nullptr;
  _msize = 0;
}


/*************************************************/
/* <----------   PRIVATE FUNCTIONS   ----------> */
/*************** vvvvvvvvvvvvvvvvv ***************/

const char cmd01[] PROGMEM = "macro";

const char cmd01desc[] PROGMEM = "\t\tHandle macro: list; read; run;";


/***** Arrays containing index of accepted ++ commands *****/
/*
 * Commands without parameters require casting to a pointer
 * requiring a char* parameter. The functon is called with
 * NULL by the command processor. 
 */
const macroHandlerPM::macCmdTxtIdx macroHandlerPM::macCmdIndex [] PROGMEM = {
  { cmd01, (0x00 | MACR_FUNC_TYPE_PARAM), cmd01desc }
};


const macroHandlerPM::macCmdRecChar macroHandlerPM::macCmdHidxChar [] = {
  { &macroHandlerPM::macro_h    }
};


/***** Extract command and pass to handler *****/
int macroHandlerPM::getHelpInfo(char * token, bool find) {

  uint8_t cmdIdxCnt = sizeof(macCmdIndex)/sizeof(*macCmdIndex);

#ifdef DEBUG_MACR_CMD_HANDLER
  if (find) {
    DB_PRINT(F("searching for: "), token);
  }
#endif

  int idx = 0;

  if (!find) dataPort.println(F("\nMacro (PM) handler:\n"));

  for (uint8_t i=0; i<cmdIdxCnt; i++) {
    if (find) {
      if ( (strcasecmp_P(token, pgm_read_ptr(&(macCmdIndex[i].token)) ) == 0 ) ) {
        idx = pgm_read_byte(&(macCmdIndex[i].idx));
      }
    }
    if (!find || idx) _util.printHelpLine( (const __FlashStringHelper *) pgm_read_ptr(&(macCmdIndex[i].token)), (const __FlashStringHelper *) pgm_read_ptr(&(macCmdIndex[i].desc)) );
    if (idx) return idx;
  }

  return -1;

}


/***** Extract command and pass to handler *****/
int macroHandlerPM::runCmd(char * cmd, char * params) {

  uint8_t paramCmdCnt = sizeof(macCmdHidxChar)/sizeof(*macCmdHidxChar);
  uint8_t cmdIdxCnt = sizeof(macCmdIndex)/sizeof(*macCmdIndex);

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
    if ( (strcasecmp_P(cmd, pgm_read_ptr(&(macCmdIndex[i].token)) ) == 0 ) ) {
      idx = pgm_read_byte(&(macCmdIndex[i].idx));
      break;
    }
  }

  // Handler not found
  if (!idx) {
    // If command not found then return
    #ifdef DEBUG_MAPM_CMD_HANDLER
      DB_PRINT(F("Handler not found!"), "");
    #endif
    return -1;
  }

  // Debug info
  #ifdef DEBUG_MAPM_CMD_HANDLER
    DB_PRINT(F("found handler for: "), cmd);
    DB_PRINT(F("Idx dec: "), idx);
  #endif

  // Run the handler

  // Handler accepts parameters
    if (idx & MACR_FUNC_TYPE_PARAM) {
    idx = idx & 0x3F;
    if (idx > paramCmdCnt) return -1;   // Invalid index
    void (macroHandlerPM::* mpc)(char *);
//     void *mpcx;
//     mpcx = (void*)pgm_read_ptr(&(comCmdHidxChar[idx].handler));
//     memcpy( &mpc, &mpcx, sizeof(mpc) );
    mpc = macCmdHidxChar[idx].handler;

    // Call handler with parameters specified
    (*this.*mpc)(params);

    #ifdef DEBUG_MAPM_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }

  // Handler does not accept parameters
/*
  if (idx & MACR_FUNC_TYPE_VOID){
    idx = idx & 0x3F;
    if (idx > voidCmdCnt) return -1;  // Invalid index
    void (macroHandlerPM::* mpv)();
//    void *mpvx;
//    mpvx = (void*)pgm_read_ptr(&(comCmdHidxVoid[idx].handler));
//    memcpy( &mpv, &mpvx, sizeof(mpv) );
    mpv = macCmdHidxVoid[(idx & 0x3F)].handler;

    // Call handler with parameters specified
    (*this.*mpv)();
    #ifdef DEBUG_MACR_CMD_HANDLER
      DB_PRINT(F("handler done."),"");
    #endif
    return 0;
  }
*/

  // No valid command found
  return -1;
}


bool macroHandlerPM::macroExists(uint8_t mnum){
  const char * macro;
  if (mnum>9) return false;
  macro = (const char *)pgm_read_ptr(macros + mnum);
  if (strlen_P(macro) > 0) return true;
  return false;
}


void macroHandlerPM::macro_h(char *params) {
  long int mnum;
  char * param1;
  char * param2;
  char * endptr;

  if (params != NULL) {
    param1 = strtok(params, " ,\t");
    param2 = strtok(NULL, " ,\t");
//    char buffer[48];

    #ifdef DEBUG_MACRO_PM_MODULE
      DB_PRINT(F("Param1: "), param1);
      DB_PRINT(F("Param2: "), param2);
    #endif

    if (strncasecmp(param1, "list", 3)==0){
      listMacros();
      return;
    }

    if (!param2) {
      _util.errorMsg(2);
      return;
    }

    mnum = strtol(param2, &endptr, 10);

    if (mnum>9) {
      _util.errorMsg(2);
      return;
    }

    if (macroExists(mnum)) {

      if (strncasecmp(param1, "run", 3)==0){
        if (macroExists(mnum)) {
         runMacro_h(mnum);
         #ifdef DEBUG_MACRO_PM_MODULE
            DB_PRINT(F("Done."), "");
         #endif
         return;
        }
      }

      if ( (strncasecmp(param1, "show", 4)==0) ){
        #ifdef DEBUG_MACRO_PM_MODULE
          size_t bytecnt = showMacro(mnum);
          DB_PRINT(F("Bytes read: "), bytecnt);
        #else
          showMacro(mnum);
        #endif
        return;
      }

    }else{
      _util.errorMsg(2);
      return;
    }

  }else{
    // No parameters given so just list available macro numbers
    listMacros();
    return;
  }
  
}


/***** List all configured macrso by number *****/
void macroHandlerPM::listMacros() {
  for (int i = 0; i < 10; i++) {
    if (macroExists(i)) {
      DATA_RAW_PRINT(i);
      DATA_RAW_PRINT(" ");
    }
  }
  DATA_RAW_PRINTLN();
}


void macroHandlerPM::runMacro_h(long int idx){

  _currentMacro = (char *)pgm_read_ptr(macros + idx);
  _msize = strlen_P(_currentMacro);

  #ifdef DEBUG_MACRO_PM_MODULE
    char c;
    DB_PRINT(F("Running macro: "), idx);
    for (size_t i=0; i<_msize; i++){
      c = pgm_read_byte_near(_currentMacro + i);
      DB_DATA_WRITE(c);
    }
  #endif

  #ifdef DEBUG_MACRO_PM_MODULE
    DB_PRINT(F("Done."), "");
  #endif

}


/***** Output the macro content *****/
size_t macroHandlerPM::showMacro(long int idx) {
  char c;
  const char * macro = (const char *)pgm_read_ptr(macros + idx);
  size_t msize = strlen_P(macro);

  #ifdef DEBUG_MACRO_PM_MODULE
    DB_PRINT(F("Reading macro: "), idx);
    DB_PRINT(F("Macro length:  "), msize);
  #endif

  if ( (idx < 10) && (msize) ) {

    // Read and show the macro
    for (size_t i=0; i<msize; i++){
      c = pgm_read_byte_near(macro + i);
      DATA_WRITE(c);
    }

    #ifdef DEBUG_MACRO_PM_MODULE
      DB_PRINT(F("Done."), "");
    #endif

    return msize;

  }

  return 0;
}


#endif  // USE_MACRO_HANDLER_PM
