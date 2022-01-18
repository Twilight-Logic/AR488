#include <Arduino.h>
//#include <SD.h>
#include "AR488_Config.h"
#include "AR488_GPIBbus.h"

/***** AR488_GPIB.cpp, ver. 0.05.33, 07/07/2021 *****/


/****** Process status values *****/
#define OK  false
#define ERR true

/***** Control characters *****/
#define ESC  0x1B   // the USB escape char
#define CR   0xD    // Carriage return
#define LF   0xA    // Newline/linefeed
#define PLUS 0x2B   // '+' character

/***** Serial/debug port *****/
#ifdef AR_SERIAL_ENABLE
  extern Stream& dataStream;
#endif

#ifdef DB_SERIAL_ENABLE
  extern Stream& debugStream;
#endif


/***************************************/
/***** GPIB CLASS PUBLIC FUNCTIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/


/********** PUBLIC FUNCTIONS **********/

/***** Class constructor *****/
GPIBbus::GPIBbus(){
  // Default configuration values
  setDefaultCfg();
  cstate = 0;
//  addressingSuppressed = false;
//  dataContinuity = false;
  deviceAddressed = false;
  deviceAddressedState = DIDS;
}


/***** Start the bus in controller or device mode depending on config *****/
void GPIBbus::begin(){
  if (isController()) {
    startControllerMode();
  }else{
    startDeviceMode();
  }
}


/***** Stops active mode and brings control and data bus to inactive state *****/
void GPIBbus::stop(){
  cstate = 0;
  // Set control bus to idle state (all lines input_pullup)
  // Input_pullup
  setGpibState(0b00000000, 0b11111111, 1);
  // All lines HIGH
  setGpibState(0b11111111, 0b11111111, 0);
  // Set data bus to default state (all lines input_pullup)
  readyGpibDbus();
}



/***** Initialise the interface *****/
void GPIBbus::setDefaultCfg(){
  // Set default values ({'\0'} sets version string array to null)
  cfg = {false, false, 2, 0, 1, 0, 0, 0, 0, 1200, 0, {'\0'}, 0, {'\0'}, 0, 0};
}


/***** Set bus into Device mode *****/
void GPIBbus::startDeviceMode(){
  // Stop current mode
  stop();
  delayMicroseconds(200); // Allow settling time
  // Start device mode
  cfg.cmode = 1;
  // Set GPIB control bus to device idle mode
  setControls(DINI);
  // Initialise GPIB data lines (sets to INPUT_PULLUP)
  readyGpibDbus();
}


/***** Set interface into Controller mode *****/
void GPIBbus::startControllerMode(){
  // Send request to clear all devices on bus to local mode
  sendAllClear();
  // Stop current mode
  stop();
  delayMicroseconds(200); // Allow settling time
  // Start controller mode
  cfg.cmode = 2;
  // Set GPIB control bus to controller idle mode
  setControls(CINI);
  // Initialise GPIB data lines (sets to INPUT_PULLUP)
  readyGpibDbus();
  // Assert IFC to signal controller in charge (CIC)
  sendIFC();
  // Attempt to address device to listen
  if (cfg.paddr > 1) addressDevice(cfg.paddr, 0);
}


/***** Return current cinterface mode *****/
bool GPIBbus::isController(){
  if (cfg.cmode == 2) return true;
  return false;
}


/***** Detect selected pin state *****/
bool GPIBbus::isAsserted(uint8_t gpibsig){
  if (getGpibPinState(gpibsig) == LOW) return true;
  return false;
}


/***** Send the device status byte *****/
void GPIBbus::sendStatus() {
  // Have been addressed and polled so send the status byte
  if (!(cstate==DTAS)) setControls(DTAS);
  writeByte(cfg.stat, NO_EOI);
  setControls(DIDS);
  // Clear the SRQ bit
  cfg.stat = cfg.stat & ~0x40;
  // De-assert the SRQ signal
  clrSrqSig();
}


/***** Set the status byte *****/
void GPIBbus::setStatus(uint8_t statusByte){
  cfg.stat = statusByte;
  if (statusByte & 0x40) {
    // If SRQ bit is set then assert the SRQ signal
    setSrqSig();
  } else {
    // If SRQ bit is NOT set then de-assert the SRQ signal
    clrSrqSig();
  }
}


/***** Send IFC *****/
void GPIBbus::sendIFC(){
  // Assert IFC
  setGpibState(0b00000000, 0b00000001, 0);
  delayMicroseconds(150);
  // De-assert IFC
  setGpibState(0b00000001, 0b00000001, 0);
}


/***** Send SDC GPIB command *****/
bool GPIBbus::sendSDC(){
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendSDC: sending SDC..."));
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendSDC: failed to address the device."));
#endif
    return ERR;
  }
  // Send SDC to currently addressed device
  if (sendCmd(GC_SDC)) {
#ifdef DEBUG_GPIB_COMMANDS    
     dataStream.println(F("GPIBbus::sendSDC: failed to send SDC to device"));
#endif
     return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendSDC: failed to unlisten the GPIB bus"));
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendSDC: done."));
#endif
  return OK;
}


/***** Send LLO GPIB command *****/
bool GPIBbus::sendLLO(){
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendLLO: sending LLO..."));
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendLLO: failed to address the device."));
#endif
    return ERR;
  }
  // Send LLO to currently addressed device
  if (sendCmd(GC_LLO)) {
#ifdef DEBUG_GPIB_COMMANDS    
     dataStream.println(F("GPIBbus::sendLLO: failed to send LLO to device"));
#endif
     return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendLLO: failed to unlisten the GPIB bus"));
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendLLO: done."));
#endif
  return OK;
}


/***** Send LOC GPIB command *****/
bool GPIBbus::sendGTL(){
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendLOC: sending LOC..."));
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendLOC: failed to address the device."));
#endif
    return ERR;
  }
  // Send GTL
  if (sendCmd(GC_GTL)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendLOC: failed to send LOC to device"));
#endif
    return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendLOC: failed to unlisten the GPIB bus"));
#endif
    return ERR;  
  }
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendLOC: done."));
#endif
  return OK;
}


/***** Send GET (trigger) command *****/
bool GPIBbus::sendGET(uint8_t addr){
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendGET: sending GET..."));
#endif
  if (addressDevice(addr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendGET: failed to address the device."));
#endif
    return ERR;
  }
  // Send GET
  if (sendCmd(GC_GET)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendGET: failed to send LOC to device"));
#endif
    return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendGET: failed to unlisten the GPIB bus"));
#endif
    return ERR;  
  }
#ifdef DEBUG_GPIB_COMMANDS    
  dataStream.println(F("GPIBbus::sendGET: done."));
#endif
  return OK;
}


/***** Send request to clear to all devices to local *****/
void GPIBbus::sendAllClear(){
  // Un-assert REN
  setControlVal(0b00100000, 0b00100000, 0);
  delay(40);
  // Simultaneously assert ATN and REN
  setControlVal(0b00000000, 0b10100000, 0);
  delay(40);
  // Unassert ATN
  setControlVal(0b10000000, 0b10000000, 0);
}


/***** Request device to talk *****/
bool GPIBbus::sendMTA(){
debugStream.print(F("GPIBbus::sendMTA: address device: "));
debugStream.println(cfg.paddr);
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  if (cstate!=CCMS) setControls(CCMS);
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  if (addressDevice(cfg.paddr, 1)) return ERR;
  return OK;
}


/***** Request device to listen *****/
bool GPIBbus::sendMLA(){
debugStream.print(F("GPIBbus::sendMTA: address device: "));
debugStream.println(cfg.paddr);
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  if (cstate!=CCMS) setControls(CCMS);
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  if (addressDevice(cfg.paddr, 0)) return ERR;
  return OK;
}

    
/***** Send secondary address command *****/
bool GPIBbus::sendMSA(uint8_t addr) {
  // Send address
  if (sendCmd(addr)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendMSA: failed to send MSA to device"));
#endif
    return ERR;
  }
  // Unassert ATN
  setControlVal(0b10000000, 0b10000000, 0);
  return OK;
}


/***** Send untalk (SAD mode) *****/
bool GPIBbus::sendUNT(){
  if (sendCmd(GC_UNT)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendUNT: failed to send UNT to device"));
#endif
    return ERR;
  }
  setControls(CIDS);
  deviceAddressed = false;
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  return OK;
}


/***** Send unlisten *****/
bool GPIBbus::sendUNL(){
  if (sendCmd(GC_UNL)) {
#ifdef DEBUG_GPIB_COMMANDS    
    dataStream.println(F("GPIBbus::sendUNL: failed to send UNL to device"));
#endif
    return ERR;
  }
  setControls(CIDS);
  deviceAddressed = false;
debugStream.print(F("GPIBbus::sendMTA: cstate: "));
debugStream.println(cstate, HEX);
  return OK;
}


/*****  Send a single byte GPIB command *****/
bool GPIBbus::sendCmd(uint8_t cmdByte){
  bool stat = false;
  // Set lines for command and assert ATN
  if (cstate!=CCMS) setControls(CCMS);
  // Send the command
  stat = writeByte(cmdByte, NO_EOI);
#if defined (DEBUG_GPIBbus_RECEIVE) || defined (DEBUG_GPIBbus_SEND)
  if (stat) { // true = error
    dataStream.print(F("GPIBbus::sendCmd: Failed to send command "));
    dataStream.print(cmdByte, HEX);
    dataStream.print(" to device ");
    dataStream.println(cfg.paddr);
  }
#endif  
  return stat ? ERR : OK;
}


/***** Receive data from the GPIB bus ****/
/*
 * Readbreak:
 * 7 - command received via serial
 */
bool GPIBbus::receiveData(Stream& dataStream, bool detectEoi, bool detectEndByte, uint8_t endByte) {

  uint8_t r = 0; //, db;
  uint8_t bytes[3] = {0};
  uint8_t eor = cfg.eor&7;
  int x = 0;
  bool readWithEoi = false;
  bool eoiDetected = false;

  endByte = endByte;  // meaningless but defeats vcompiler warning!

  // Reset transmission break flag
  txBreak = 0;

  // EOI detection required ?
  if (cfg.eoi || detectEoi || (cfg.eor==7)) readWithEoi = true;    // Use EOI as terminator

  // Set up for reading in Controller mode
  if (cfg.cmode == 2) {   // Controler mode
/*    
    // Address device to talk
    if (addressDevice(cfg.paddr, 1)) {
#ifdef DEBUG_GPIBbus_RECEIVE
      debugStream.print(F("GPIBbus::receiveData: Failed to address device "));
      debugStream.print(cfg.paddr);
      debugStream.println(F(" to talk"));
#endif
    }
*/   
    // Wait for instrument ready
//    waitOnPinState(HIGH, NRFD, cfg.rtmo); // readByte already does this
    // Set GPIB control lines to controller read mode
    setControls(CLAS);
    
  // Set up for reading in Device mode
  } else {  // Device mode
    // Set GPIB controls to device read mode
    setControls(DLAS);
    readWithEoi = true;  // In device mode we read with EOI by default
  }

  
#ifdef DEBUG_GPIBbus_RECEIVE
  debugStream.println(F("GPIBbus::receiveData: Start listen ->"));
  debugStream.println(F("Before loop flags:"));
  debugStream.print(F("TRNb: "));
  debugStream.println(txBreak);
  debugStream.print(F("rEOI: "));
  debugStream.println(readWithEoi);
  debugStream.print(F("ATN:  "));
  debugStream.println(isAsserted(ATN) ? 1 : 0);
#endif

  // Ready the data bus
  readyGpibDbus();

  // Perform read of data (r=0: data read OK; r>0: GPIB read error);
  while (r == 0) {

    // Tranbreak > 0 indicates break condition
    if (txBreak) break;

    // ATN asserted
    if (isAsserted(ATN)) break;

    // Read the next character on the GPIB bus
    r = readByte(&bytes[0], readWithEoi, &eoiDetected);

    // If ATN asserted then break here
    if (isAsserted(ATN)) break;

#ifdef DEBUG_GPIBbus_RECEIVE
    debugStream.print(bytes[0], HEX), debugStream.print(' ');
#else
    // Output the character to the serial port
    dataStream.print((char)bytes[0]);
#endif

    // Byte counter
    x++;

    // EOI detection enabled and EOI detected?
    if (readWithEoi) {
      if (eoiDetected) break;
    }else{
      // Has a termination sequence been found ?
      if (detectEndByte) {
        if (r == endByte) break;
      }else{
        if (isTerminatorDetected(bytes, eor)) break;
      }
    }

    // Stop on timeout
    if (r > 0) break;

    // Shift last three bytes in memory
    bytes[2] = bytes[1];
    bytes[1] = bytes[0];
  }

#ifdef DEBUG_GPIBbus_RECEIVE
  debugStream.println();
  debugStream.println(F("After loop flags:"));
  debugStream.print(F("ATN: "));
  debugStream.println(isAsserted(ATN));
  debugStream.print(F("TMO:  "));
  debugStream.println(r);
  debugStream.print(F("Bytes read: "));
  debugStream.println(x);
  debugStream.println(F("GPIBbus::receiveData: <- End listen."));
#endif

  // Detected that EOI has been asserted
  if (eoiDetected) {
#ifdef DEBUG_GPIBbus_RECEIVE
    dataStream.println(F("GPIBbus::receiveData: EOI detected!"));
#endif
    // If eot_enabled then add EOT character
    if (cfg.eot_en) dataStream.print(cfg.eot_ch);
  }

  // Verbose timeout error
#ifdef DEBUG_GPIBbus_RECEIVE
  if (r > 0) {
    debugStream.println(F("GPIBbus::receiveData: Timeout waiting for sender!"));
    debugStream.println(F("GPIBbus::receiveData: Timeout waiting for transfer to complete!"));
  }
#endif

  // Return controller to idle state
  if (cfg.cmode == 2) {

/*
    // Untalk bus and unlisten controller
    if (unAddressDevice()) {
#ifdef DEBUG_GPIBbus_RECEIVE
      debugStream.println(F("GPIBbus::receiveData: Failed to untalk bus"));
#endif
    }
*/

    // Set controller back to idle state
    setControls(CIDS);

  } else {
    // Set device back to idle state
    setControls(DIDS);
  }

  // Reset break flag
  if (txBreak) txBreak = false;

#ifdef DEBUG_GPIBbus_RECEIVE
  debugStream.println(F("GPIBbus::receiveData: done."));
#endif

  if (r > 0) return ERR;

  return OK;

}


/***** Send a series of characters as data to the GPIB bus *****/
void GPIBbus::sendData(char *data, uint8_t dsize) {

  bool err = false;

/* Shouldn't get gere!
  // If lon is turned on we cannot send data so exit
  if (isRO) return;
*/

  // Controler can unlisten bus and address devices
  if (cfg.cmode == 2) {
/*
    if (!addressingSuppressed) {
      // Address device to listen
      if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIBbus_SEND
        debugStream.print(F("GPIBbus::sendData: failed to address device "));
        debugStream.print(cfg.paddr);
        debugStream.println(F(" to listen"));
#endif
        return;
      }
    }

    addressingSuppressed = dataContinuity;

#ifdef DEBUG_GPIBbus_SEND
    debugStream.println(F("GPIBbus::sendData: device addressed."));
#endif
*/
    // Set control lines to write data (ATN unasserted)
    setControls(CTAS);

  } else {
    setControls(DTAS);
  }
#ifdef DEBUG_GPIBbus_SEND
  debugStream.println(F("GPIBbus::sendData: write data mode is set."));
  debugStream.println(F("GPIBbus::sendData: begin send loop->"));
#endif

  // Write the data string
  for (int i = 0; i < dsize; i++) {
    // If EOI asserting is on
    if (cfg.eoi) {
      // Send all characters
      err = writeByte(data[i], NO_EOI);
    } else {
      // Otherwise ignore non-escaped CR, LF and ESC
      if ((data[i] != CR) || (data[i] != LF) || (data[i] != ESC)) err = writeByte(data[i], NO_EOI);
    }
#ifdef DEBUG_GPIBbus_SEND
    debugStream.print(data[i]);
#endif
    if (err) break;
  }

#ifdef DEBUG_GPIBbus_SEND
  debugStream.println("GPIBbus::sendData: <-End of send loop.");
#endif

//  if (!err  && !dataContinuity) {
  if (!err) {
    // Write terminators according to EOS setting
    // Do we need to write a CR?
    if ((cfg.eos & 0x2) == 0) {
      writeByte(CR, NO_EOI);
#ifdef DEBUG_GPIBbus_SEND
      debugStream.println(F("GPIBbus::sendData: appended CR"));
#endif
    }
    // Do we need to write an LF?
    if ((cfg.eos & 0x1) == 0) {
      writeByte(LF, NO_EOI);
#ifdef DEBUG_GPIBbus_SEND
      debugStream.println(F("GPIBbus::sendData: appended LF"));
#endif
    }
  }

  // If EOI enabled and no more data to follow then assert EOI
//  if (cfg.eoi && !dataContinuity) {
  if (cfg.eoi) {
    setGpibState(0b00010000, 0b00010000, 1);
    setGpibState(0b00000000, 0b00010000, 0);
    delayMicroseconds(40);
    setGpibState(0b00010000, 0b00010000, 0);
#ifdef DEBUG_GPIBbus_SEND
    debugStream.println(F("Asserted EOI"));
#endif
  }

  if (cfg.cmode == 2) {   // Controller mode
/*    
    if (!err) {
      if (!addressingSuppressed) {
        // Untalk controller and unlisten bus
        if (unAddressDevice()) {
#ifdef DEBUG_GPIBbus_SEND
          debugStream.println(F("GPIBbus::sendData: failed to unlisten bus"));
#endif
        }

#ifdef DEBUG_GPIBbus_SEND
        debugStream.println(F("GPIBbus::sendData: unlisten done"));
#endif
      }
    }
*/
    // Controller - set lines to idle?
    setControls(CIDS);

  }else{    // Device mode
    // Set control lines to idle
    setControls(DIDS);
  }

#ifdef DEBUG_GPIBbus_SEND
    debugStream.println(F("GPIBbus::sendData: done."));
#endif
 
}



/**************************************************/
/***** FUCTIONS TO READ/WRITE DATA TO STORAGE *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

#ifdef EN_STORAGE

  bool GPIBbus::receiveData(iostream& fileStream, bool detectEoi, bool detectEndByte, uint8_t endByte){
      
  }

  void GPIBbus::sendData(iostream& fileStream){
    
  }

#endif

/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** FUCTIONS TO READ/WRITE DATA TO STORAGE *****/
/**************************************************/



/***** Signal to break a GPIB transmission *****/
void GPIBbus::signalBreak(){
  txBreak = true;
}


/***** Control the GPIB bus - set various GPIB states *****/
/*
 * state is a predefined state (CINI, CIDS, CCMS, CLAS, CTAS, DINI, DIDS, DLAS, DTAS);
 * Bits control lines as follows: 8-ATN, 7-SRQ, 6-REN, 5-EOI, 4-DAV, 3-NRFD, 2-NDAC, 1-IFC
 * setGpibState byte1 (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
 * setGpibState byte2 (mask)     : 0=unaffected, 1=enabled
 * setGpibState byte3 (mode)     : 0=set pin state, 1=set pin direction
 */
void GPIBbus::setControls(uint8_t state) {

  // Switch state
  switch (state) {

    // Controller states
    case CINI:  // Initialisation
      // Set pin direction
      setGpibState(0b10111000, 0b11111111, 1);
      // Set pin state
      setGpibState(0b11011111, 0b11111111, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
  #ifdef SN7516X_DC
        digitalWrite(SN7516X_DC,LOW);
  #endif
  #ifdef SN7516X_SC
        digitalWrite(SN7516X_SC,HIGH);
  #endif
#endif      
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Initialised GPIB control mode"));
#endif
      break;

    case CIDS:  // Controller idle state
      setGpibState(0b10111000, 0b10011110, 1);
      setGpibState(0b11011111, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines to idle state"));
#endif
      break;

    case CCMS:  // Controller active - send commands
      setGpibState(0b10111001, 0b10011111, 1);
      setGpibState(0b01011111, 0b10011111, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines for sending a command"));
#endif
      break;

    case CLAS:  // Controller - read data bus
      // Set state for receiving data
      setGpibState(0b10100110, 0b10011110, 1);
      setGpibState(0b11011000, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines for reading data"));
#endif
      break;

    case CTAS:  // Controller - write data bus
      setGpibState(0b10111001, 0b10011110, 1);
      setGpibState(0b11011111, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines for writing data"));
#endif
      break;

    /* Bits control lines as follows: 8-ATN, 7-SRQ, 6-REN, 5-EOI, 4-DAV, 3-NRFD, 2-NDAC, 1-IFC */

    // Listener states
    case DINI:  // Listner initialisation
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
  #ifdef SN7516X_DC
        digitalWrite(SN7516X_DC,HIGH);
  #endif
  #ifdef SN7516X_SC
        digitalWrite(SN7516X_SC,LOW);
  #endif
#endif      
      setGpibState(0b00000000, 0b11111111, 1);
      setGpibState(0b11111111, 0b11111111, 0);
      // Set data bus to idle state
      readyGpibDbus();
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Initialised GPIB listener mode"));
#endif
      break;

    case DIDS:  // Device idle state
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
      setGpibState(0b00000000, 0b00001110, 1);
      setGpibState(0b11111111, 0b00001110, 0);
      // Set data bus to idle state
      readyGpibDbus();
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines to idle state"));
#endif
      break;

    case DLAS:  // Device listner active (actively listening - can handshake)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
      setGpibState(0b00000110, 0b00011110, 1);
      setGpibState(0b11111001, 0b00011110, 0);
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines to idle state"));
#endif
      break;

    case DTAS:  // Device talker active (sending data)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
      setGpibState(0b00011000, 0b00011110, 1);
      setGpibState(0b11111001, 0b00011110, 0);
#ifdef DEBUG_GPIBbus_CONTROL
      debugStream.println(F("Set GPIB lines for listening as addresed device"));
#endif
      break;
#ifdef DEBUG_GPIBbus_CONTROL
    default:
      // Should never get here!
      //      setGpibState(0b00000110, 0b10111001, 0b11111111);
      debugStream.println(F("Unknown GPIB state requested!"));
#endif
  }

  // Save state
  cstate = state;

  // GPIB bus delay (to allow state to settle)
//  delayMicroseconds(AR488.tmbus);

}


/***** Set GPI control state using numeric input (xdiag_h) *****/
void GPIBbus::setControlVal(uint8_t value, uint8_t mask, uint8_t mode){
  setGpibState(value, mask, mode);
}


/***** Set GPIB data bus to specific value (xdiag_h) *****/
void GPIBbus::setDataVal(uint8_t value){
  setGpibDbus(value);
}


/***** Flag more data to come - suppress addressing *****/ 
/*
void GPIBbus::setDataContinuity(bool flag){
  dataContinuity = flag;
}
*/


/***** Flag that addressing should be suppressed *****/
/*
void GPIBbus::setAddressingSuppressed(bool flag){
  addressingSuppressed = flag;
}
*/

/***** Unaddress device *****/
bool GPIBbus::unAddressDevice() {
  // De-bounce
  delayMicroseconds(30);
  // Utalk/unlisten
  if (sendCmd(GC_UNL)) return ERR;
  if (sendCmd(GC_UNT)) return ERR;
  // Clear flag
  deviceAddressed = false;
#ifdef DEBUG_GPIBbus_CONTROL
  debugStream.println(F("unAddressDevice: done."));
#endif
  return OK;
}


/***** Untalk bus then address a device *****/
/*
 * talk: false=listen; true=talk;
 */
bool GPIBbus::addressDevice(uint8_t addr, bool talk) {
//  uint8_t saddr = 0;
  if (sendCmd(GC_UNL)) return ERR;
#ifdef DEBUG_GPIBbus_CONTROL
  debugStream.print(F("addressDevice: "));
  debugStream.println(addr);  
#endif
  if (talk) {
    // Device to talk, controller to listen
    if (sendCmd(GC_TAD + addr)) return ERR;
//    if (sendCmd(GC_LAD + cfg.caddr)) return ERR;
  } else {
    // Device to listen, controller to talk
    if (sendCmd(GC_LAD + addr)) return ERR;
//    if (sendCmd(GC_TAD + cfg.caddr)) return ERR;
  }

  // Set flag
  deviceAddressed = true;

  return OK;
}


/***** Returns status of device addressing *****/
/*
 * true = device addressed; false = device is not addressed
 */
bool GPIBbus::isDeviceAddressed(){
  return deviceAddressed;
}


void GPIBbus::setDeviceAddressedState(uint8_t state){
  // Valid state supplied
  if (state==DLAS || state==DTAS) {
    deviceAddressedState = state;
    return;
  }
  // Otherwise set to idle
  deviceAddressedState = DIDS;
}


/***** Device is addressed to listen? *****/
bool GPIBbus::isDeviceAddressedToListen(){
  if (deviceAddressedState == DLAS) return true;
  return false;
}


/***** Device is addressed to talk? *****/
bool GPIBbus::isDeviceAddressedToTalk(){
  if (deviceAddressedState == DTAS) return true;
  return false;
}


/***** Device is not addressed? *****/
bool GPIBbus::isDeviceNotAddressed(){
  if (deviceAddressedState == DIDS) return true;
  return false;
}


/***** Read a SINGLE BYTE of data from the GPIB bus using 3-way handshake *****/
/*
 * (- this function is called in a loop to read data    )
 * (- the GPIB bus must already be configured to listen )
 */
uint8_t GPIBbus::readByte(uint8_t *db, bool readWithEoi, bool *eoi) {

  unsigned long startMillis = millis();
  unsigned long currentMillis = startMillis + 1;
  const unsigned long timeval = cfg.rtmo;
  uint8_t stage = 4;

  bool atnStat = isAsserted(ATN);
  *eoi = false;

  // Wait for interval to expire
  while ( (unsigned long)(currentMillis - startMillis) < timeval ) {

    if (cfg.cmode == 1) {
      // If IFC has been asserted then abort
      if (isAsserted(IFC)) {
#ifdef DEBUG_GPIBbus_RECEIVE
        dataStream.println(F("GPIBbus::readByte: IFC detected]"));
#endif
        stage = 1;
        break;    
      }

      // ATN unasserted during handshake - not ready yet so abort (and exit ATN loop)
      if ( atnStat && !isAsserted(ATN) ){
        stage = 2;
        break;
      }

    }  

    if (stage == 4) {
      // Unassert NRFD (we are ready for more data)
      setGpibState(0b00000100, 0b00000100, 0);
      stage = 6;
    }

    if (stage == 6) {
      // Wait for DAV to go LOW indicating talker has finished setting data lines..
      if (digitalRead(DAV) == LOW) {
        // Assert NRFD (Busy reading data)
        setGpibState(0b00000000, 0b00000100, 0);
        stage = 7;
      }
    }

    if (stage == 7) {
      // Check for EOI signal
      if (readWithEoi && isAsserted(EOI)) *eoi = true;
      // read from DIO
      *db = readGpibDbus();
      // Unassert NDAC signalling data accepted
      setGpibState(0b00000010, 0b00000010, 0);
      stage = 8;
    }

    if (stage == 8) {
      // Wait for DAV to go HIGH indicating data no longer valid (i.e. transfer complete)
      if (digitalRead(DAV) == HIGH) {
        // Re-assert NDAC - handshake complete, ready to accept data again
        setGpibState(0b00000000, 0b00000010, 0);
        stage = 9;
        break;     
      }
    }

    // Increment time
    currentMillis = millis();

  }

  // Completed
  if (stage == 9) return 0;

  if (stage==1) return 4;
  if (stage==2) return 3;
  
  // Otherwise return stage
#ifdef DEBUG_GPIBbus_RECEIVE
  if ( (stage==4) || (stage==8) ) {
    debugStream.println(F("DAV timout!"));
  }else{
    debugStream.print(F("Error: "));
    debugStream.println(stage);
  }
#endif

  return stage;

}


uint8_t GPIBbus::writeByte(uint8_t db, bool isLastByte) {
  unsigned long startMillis = millis();
  unsigned long currentMillis = startMillis + 1;
  const unsigned long timeval = cfg.rtmo;
  uint8_t stage = 4;

  // Wait for interval to expire
  while ( (unsigned long)(currentMillis - startMillis) < timeval ) {

    if (cfg.cmode == 1) {
      // If IFC has been asserted then abort
      if (isAsserted(IFC)) {
        setControls(DLAS);       
#ifdef DEBUG_GPIBbus_SEND
        debugStream.println(F("GPIBbus::writeByte: IFC detected!"));
#endif
        stage = 1;
        break;    
      }

      // If ATN has been asserted we need to abort and listen
      if (isAsserted(ATN)) {
        setControls(DLAS);       
#ifdef DEBUG_GPIBbus_SEND
        debugStream.println(F("gpibBus::writeByte: ATN detected!"));
#endif
        stage = 2;
        break;
      }
    }

    // Wait for NDAC to go LOW (indicating that devices (stage==4) || (stage==8) ) are at attention)
    if (stage == 4) {
      if (digitalRead(NDAC) == LOW) stage = 5;
    }

    // Wait for NRFD to go HIGH (indicating that receiver is ready)
    if (stage == 5) {
      if (digitalRead(NRFD) == HIGH) stage = 6;
    }

    if (stage == 6){
      // Place data on the bus
      setGpibDbus(db);
      if (cfg.eoi && isLastByte) {
        // If EOI enabled and this is the last byte then assert DAV and EOI
#ifdef DEBUG_GPIBbus_SEND
        debugStream.println(F("Asserting EOI..."));    
#endif
        setGpibState(0b00000000, 0b00011000, 0);
      }else{
        // Assert DAV (data is valid - ready to collect)
        setGpibState(0b00000000, 0b00001000, 0);
      }
      stage = 7;
    }

    if (stage == 7) {
      // Wait for NRFD to go LOW (receiver accepting data)
      if (digitalRead(NRFD) == LOW) stage = 8;
    }

    if (stage == 8) {
      // Wait for NDAC to go HIGH (data accepted)
      if (digitalRead(NDAC) == HIGH) {
        stage = 9;
        break;
      }
    }

    // Increment time
    currentMillis = millis();

  }

  // Handshake complete
  if (stage == 9) {
    if (cfg.eoi && isLastByte) {
      // If EOI enabled and this is the last byte then un-assert both DAV and EOI
      if (cfg.eoi && isLastByte) setGpibState(0b00011000, 0b00011000, 0);
    }else{
      // Unassert DAV
      setGpibState(0b00001000, 0b00001000, 0);
    }
    // Reset the data bus
    setGpibDbus(0);
    return 0;
  }

  // Otherwise timeout or ATN/IFC return stage at which it ocurred
#ifdef DEBUG_GPIBbus_SEND
  switch (stage) {
    case 4:
      debugStream.print("NDAC timeout!");
      break;
    case 5:
      debugStream.print("NRFD timout!");
      break;
    case 7:
      debugStream.print("NRFD timout!");
      break;
    case 8:
      debugStream.print("NDAC timout!");
      break;
    default:
      debugStream.print(F("Error: "));
      debugStream.println(stage);
  }
#endif

  return stage;
}


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GPIB CLASS PUBLIC FUNCTIONS *****/
/***************************************/




/***************************************/
/***** GPIB CLASS PRIVATE FUNCTIONS *****/
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/



/********** PRIVATE FUNCTIONS **********/




/***** Wait for "pin" to reach a specific state *****/
/*
 * Returns false on success, true on timeout.
 * Pin MUST be set as INPUT_PULLUP otherwise it will not change and simply time out!
 */
/*
boolean GPIBbus::waitOnPinState(uint8_t state, uint8_t pin, int interval) {

  unsigned long timeout = millis() + interval;
  bool atnStat = (isAsserted(ATN));
  while (digitalRead(pin) != state) {
    // Check timer
    if (millis() >= timeout) {
      return true;
    }
    // ATN status was asserted but now unasserted so abort
    if (atnStat && !isAsserted(ATN)) return true;
    delayMicroseconds(100);
  }
  return false;        // = no timeout therefore succeeded!
}
*/

/***** Check for terminator *****/
bool GPIBbus::isTerminatorDetected(uint8_t bytes[3], uint8_t eorSequence){
  // Look for specified terminator (CR+LF by default)
  switch (eorSequence) {
    case 0:
        // CR+LF terminator
        if (bytes[0]==LF && bytes[1]==CR) return true;
        break;
    case 1:
        // CR only as terminator
        if (bytes[0]==CR) return true;
        break;
    case 2:
        // LF only as terminator
        if (bytes[0]==LF) return true;
        break;
    case 3:
        // No terminator (will rely on timeout)
        break;
    case 4:
        // Keithley can use LF+CR instead of CR+LF
        if (bytes[0]==CR && bytes[1]==LF) return true;
        break;
    case 5:
        // Solarton (possibly others) can also use ETX (0x03)
        if (bytes[0]==0x03) return true;
        break;
    case 6:
        // Solarton (possibly others) can also use CR+LF+ETX (0x03)
        if (bytes[0]==0x03 && bytes[1]==LF && bytes[2]==CR) return true;
        break;
    default:
        // Use CR+LF terminator by default
        if (bytes[0]==LF && bytes[1]==CR) return true;
        break;
  }
  return false;
}


/***** Set the SRQ signal *****/
void GPIBbus::setSrqSig() {
  // Set SRQ line to OUTPUT HIGH (asserted)
  setGpibState(0b01000000, 0b01000000, 1);
  setGpibState(0b00000000, 0b01000000, 0);
}


/***** Clear the SRQ signal *****/
void GPIBbus::clrSrqSig() {
  // Set SRQ line to INPUT_PULLUP (un-asserted)
  setGpibState(0b00000000, 0b01000000, 1);
  setGpibState(0b01000000, 0b01000000, 0);
}


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GPIB CLASS PRIVATE FUNCTIONS *****/
/****************************************/
