#include "Utility_Helper.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                       UTILITY HELPER                        ||
||                                                             ||
\*=============================================================*/


utilityHelper::utilityHelper(){};


/***** Reset the controller *****/
/*
 * Arduinos can use the watchdog timer to reset the MCU
 * For other devices, we restart the program instead by
 * jumping to address 0x0000. This is not a hardware reset
 * and will not reset a crashed MCU, but it will re-start
 * the interface program and re-initialise all parameters. 
 */
void utilityHelper::mcuReset() {
#ifdef WDTO_1S
  params = NULL;    // Ignore any passed parameters
  // Where defined, reset controller using watchdog timeout
  unsigned long tout;
  tout = millis() + 2000;
  wdt_enable(WDTO_1S);
  while (millis() < tout) {};
  // Should never reach here....
  if (isVerb) {
    dataPort.println(F("Reset FAILED."));
  };
#elif defined (ESP32)
  ESP.restart();
#elif defined(ARDUINO_ARCH_RP2040)
  rp2040.reboot();
#elif defined(__IMXRT1062__)    // Teensy
  SCB_AIRCR = 0x05FA0004;
#elif defined(ARDUINO_ARCH_RENESAS)
  NVIC_SystemReset();
#else
  // Otherwise restart program (soft reset)
  asm volatile ("  jmp 0");
#endif
}


/***** Does the char array contain a representation of a number *****/
bool utilityHelper::isNumber(char *numstr){
  if (numstr == nullptr) return false;
  uint8_t numlen = strlen(numstr);
  if (numlen == 0) return false;
  for (uint8_t i=0; i<numlen; i++){
    if (numstr[i]<48 || numstr[i]>57) return false;
  }
  return true;
}


/***** Is parameter an x-y range string *****/
bool utilityHelper::isRange(char * rangestr, size_t rsize, unsigned long values[2] ) {
  char * fp = NULL;

  for (uint8_t i=0; i<rsize; i++) {
    if (rangestr[i] == '-') {
      if (!i) return false;
      fp = &rangestr[i+1];
      values[0] = strtoul(rangestr, NULL, 10);
      values[1] = strtoul(fp, NULL, 10);
      if (values[1] > values[0]) return true;
    }
  }
  return false;
}


bool utilityHelper::notInRange(char *param, uint16_t lowl, uint16_t higl, uint16_t &rval) {

  unsigned long val = 0;
  char* end = NULL;

  // Null string passed?
//  if (strlen(param) == 0) return true;
  if (param == nullptr) return true;


  // Is it numeric ?
  if (!isNumber(param)) return true;

  // Convert
  val = strtoul(param, &end, 10);

  // Check range
  if (val < lowl || val > higl) {
    errorMsg(2);
    if (isVerb) {
      DATA_RAW_PRINT(F("Valid range is between "));
      DATA_RAW_PRINT(lowl);
      DATA_RAW_PRINT(F(" and "));
      DATA_RAW_PRINTLN(higl);
    }
    return true;
  }
  rval = (uint16_t)val;

  return false;
}


/***** Error messsages *****/
void utilityHelper::errorMsg(int err) {
  switch (err) {
#ifdef INTERFACE_PROLOGIX
    case 1:
      DATA_RAW_PRINTLN(F("Missing parameter"));
      break;
    case 2:
      DATA_RAW_PRINTLN(F("Invalid parameter"));
      break;
    case 3:
      DATA_RAW_PRINTLN(F("Transmit failed!"));
      break;
    case 4:
      DATA_RAW_PRINTLN(F("Invalid mode."));
      break;
    case 5:
      DATA_RAW_PRINTLN(F("Out of range!"));
      break;
#endif
    default:
      DATA_RAW_PRINTLN(F("Unrecognized command!"));
  }
}


/***** Show a prompt *****/
void utilityHelper::showPrompt() {
  // Print a prompt
  DATA_RAW_PRINT("> ");
}


void utilityHelper::printHelpLine(const __FlashStringHelper * token, const __FlashStringHelper * desc) {
  dataPort.print(F("++"));
  dataPort.print(token);
  dataPort.println(desc);
}


/***** Wait for desired pin state *****/
/*
 * Asserted: true, false
// State 0 = wait for low
// * State 1 = wait for high
// * LOW = asserted, HIGH = unasserted
 * tmo = timeout (up to 32k milliseconds [32 sec])
 * Returns: 0-failed, 1-state reached within timeout
*/
bool utilityHelper::waitForPinState(uint8_t pin, bool asserted, unsigned long tmo) {
  const unsigned long timeout = millis() + tmo;
//  if (state > 1) return false;  // Invalid
  while (millis() < timeout) {
    if (asserted) {
      if (getGpibPinState(pin) == LOW) return true;
    }else{
      if (getGpibPinState(pin) == HIGH) return true;
    }
    delayMicroseconds(20);  // Wait a short while before trying again
  }
  return false;
}