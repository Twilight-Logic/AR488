#include <Arduino.h>
//#include <SD.h>
#include "AR488_Config.h"
#include "AR488_GPIBbus.h"

/***** AR488_GPIB.cpp, ver. 0.51.17, 24/12/2022 *****/


/****** Process status values *****/
#define OK  false
#define ERR true

/***** Control characters *****/
#define ESC  0x1B   // the USB escape char
#define CR   0xD    // Carriage return
#define LF   0xA    // Newline/linefeed
#define PLUS 0x2B   // '+' character

/***** Serial/debugStream port *****/
/*
#ifdef AR_SERIAL_ENABLE
  extern Stream& dataStream;
#endif

#ifdef DB_SERIAL_ENABLE
  extern Stream& debugStream;
#endif
*/

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
//  deviceAddressedState = DIDS;
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
  inputCtrl(ALL_BITS);
  clearCtrl(ALL_BITS); // HIGH
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
#if defined(AR488_MCP23S17) || defined(AR488_MCP23017)
  uint8_t mcpPinAssertedReg = 0;
  // Use MCP function to get MCP23S17 or MCP23017 pin state.
  // If interrupt flagged then update mcpPinAssertedReg register
//  if (isMcpIntFlagSet()){
//dataPort.println(F("Interrupt flagged - pin state checked"));
    // Clear mcpIntA flag
//    clearMcpIntFlag();
    // Get inverse of pin status at interrupt (0 = true [asserted]; 1 = false [unasserted])
    // Calling getMcpIntAPinState clears the interrupt
//    mcpPinAssertedReg = ~getMcpIntAPinState();
    mcpPinAssertedReg = ~getMcpIntAReg();
//dataPort.print(F("mcpPinAssertedReg: "));
//dataPort.println(mcpPinAssertedReg, BIN);
//  }
  return (mcpPinAssertedReg & (1<<gpibsig));
#else
  // Use digitalRead function to get current Arduino pin state
  return (digitalRead(gpibsig) == LOW) ? true : false;
#endif


  
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
  assertCtrl(IFC_BIT);
  delayMicroseconds(150);
  // De-assert IFC
  clearCtrl(IFC_BIT);
}


/***** Send SDC GPIB command *****/
bool GPIBbus::sendSDC(){
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("sending SDC..."),"");
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to address the device."),"");
#endif
    return ERR;
  }
  // Send SDC to currently addressed device
  if (sendCmd(GC_SDC)) {
#ifdef DEBUG_GPIB_COMMANDS
     DB_PRINT(F("failed to send SDC to device"),"");
#endif
     return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to unlisten the GPIB bus"),"");
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Send LLO GPIB command *****/
bool GPIBbus::sendLLO(){
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("sending LLO..."),"");
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to address the device."),"");
#endif
    return ERR;
  }
  // Send LLO to currently addressed device
  if (sendCmd(GC_LLO)) {
#ifdef DEBUG_GPIB_COMMANDS
     DB_PRINT(F("failed to send LLO to device"),"");
#endif
     return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to unlisten the GPIB bus"),"");
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Send LOC GPIB command *****/
bool GPIBbus::sendGTL(){
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("sending LOC..."),"");
#endif
  if (addressDevice(cfg.paddr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to address the device."),"");
#endif
    return ERR;
  }
  // Send GTL
  if (sendCmd(GC_GTL)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to send LOC to device"),"");
#endif
    return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to unlisten the GPIB bus"),"");
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Send GET (trigger) command *****/
bool GPIBbus::sendGET(uint8_t addr){
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("sending GET..."),"");
#endif
  if (addressDevice(addr, 0)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to address the device."),"");
#endif
    return ERR;
  }
  // Send GET
  if (sendCmd(GC_GET)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to send LOC to device"),"");
#endif
    return ERR;
  }
  // Unlisten bus
  if (unAddressDevice()) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to unlisten the GPIB bus"),"");
#endif
    return ERR;
  }
#ifdef DEBUG_GPIB_COMMANDS
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Send request to clear to all devices to local *****/
void GPIBbus::sendAllClear(){
  clearCtrl(REN_BIT);
  delay(40);
  assertCtrl(ATN_BIT | REN_BIT);
  delay(40);
  clearCtrl(ATN_BIT);
}


/***** Request device to talk *****/
bool GPIBbus::sendMTA(){
#ifdef DEBUG_GPIB_ADDRESSING
  DB_PRINT(F("address device: "), cfg.paddr);
//  char hexstr[4];
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),nexstr);
#endif
  if (cstate!=CCMS) setControls(CCMS);
#ifdef DEBUG_GPIB_ADDRESSING
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),hexstr);
#endif
  if (addressDevice(cfg.paddr, 1)) return ERR;
#ifdef DEBUG_GPIB_ADDRESSING
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Request device to listen *****/
bool GPIBbus::sendMLA(){
#ifdef DEBUG_GPIB_ADDRESSING
  DB_PRINT(F("address device: "), cfg.paddr);
//  char hexstr[4];
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),nexstr);
#endif
  if (cstate!=CCMS) setControls(CCMS);
#ifdef DEBUG_GPIB_ADDRESSING
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),hexstr);
#endif
  if (addressDevice(cfg.paddr, 0)) return ERR;
#ifdef DEBUG_GPIB_ADDRESSING
  DB_PRINT(F("done."),"");
#endif
  return OK;
}


/***** Send secondary address command *****/
bool GPIBbus::sendMSA(uint8_t addr) {
  // Send address
  if (sendCmd(addr)) {
#ifdef DEBUG_GPIB_COMMANDS
    DB_PRINT(F("failed to send MSA to device"),"");
#endif
    return ERR;
  }
  // Unassert ATN
  clearCtrl(ATN_BIT);
  return OK;
}


/***** Send untalk (SAD mode) *****/
bool GPIBbus::sendUNT(){
  if (sendCmd(GC_UNT)) {
#ifdef DEBUG_GPIB_ADDRESSING
    DB_PRINT(F("failed to send UNT to device"),"");
#endif
    return ERR;
  }
  setControls(CIDS);
  deviceAddressed = false;
#ifdef DEBUG_GPIB_ADDRESSING
//  char hexstr[4];
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),nexstr);
#endif
  return OK;
}


/***** Send unlisten *****/
bool GPIBbus::sendUNL(){
  if (sendCmd(GC_UNL)) {
#ifdef DEBUG_GPIB_ADDRESSING
    DB_PRINT(F("failed to send UNL to device"),"");
#endif
    return ERR;
  }
  setControls(CIDS);
  deviceAddressed = false;
#ifdef DEBUG_GPIB_ADDRESSING
//  char hexstr[4];
//  sprintf(hexstr,"%02X", cstate);
//  DB_PRINT(F("cstate: "),nexstr);
#endif
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
    char hexstr[4];
    sprintf(hexstr,"%02X", cmdByte);
    DB_PRINT(F("Failed to send command "), hexstr);
    DB_PRINT(F("  to device "), cfg.paddr);
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

    // Address device to talk
    if (addressDevice(cfg.paddr, 1)) {
#ifdef DEBUG_GPIBbus_RECEIVE
      DB_PRINT(F("Failed to address device to talk: "), cfg.paddr);
#endif
    }

    // Wait for instrument ready
    // Set GPIB control lines to controller read mode
    setControls(CLAS);

  // Set up for reading in Device mode
  } else {  // Device mode
    // Set GPIB controls to device read mode
    setControls(DLAS);
    readWithEoi = true;  // In device mode we read with EOI by default
  }

#ifdef DEBUG_GPIBbus_RECEIVE
  DB_PRINT(F("Start listen ->"),"");
  DB_PRINT(F("Before loop flags:"),"");
  DB_PRINT(F("TRNb: "), txBreak);
  DB_PRINT(F("rEOI: "), readWithEoi);
//  DB_PRINT(F("ATN:  "), (isAsserted(ATN ? 1 : 0));
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

    if (isAsserted(ATN)) r = 2;

    // If IFC or ATN asserted then break here
    if ( (r==1) || (r==2) ) break;

    // If successfully received character
    if (r==0) {
#ifdef DEBUG_GPIBbus_RECEIVE
      DB_HEX_PRINT(bytes[0]);
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

      // Shift last three bytes in memory
      bytes[2] = bytes[1];
      bytes[1] = bytes[0];
    }else{
      // Stop (error or timeout)
      break;
    }
  }

#ifdef DEBUG_GPIBbus_RECEIVE
  DB_RAW_PRINTLN();
  DB_PRINT(F("After loop flags:"),"");
//  DB_PRINT(F("ATN: "), (isAsserted(ATN ? 1 : 0));
  DB_PRINT(F("TMO: "), r);
  DB_PRINT(F("Bytes read:  "), x);
  DB_PRINT(F("<- End listen."),"");
#endif

  // Detected that EOI has been asserted
  if (eoiDetected) {
#ifdef DEBUG_GPIBbus_RECEIVE
    DB_PRINT(F("EOI detected!"),"");
#endif
    // If eot_enabled then add EOT character
    if (cfg.eot_en) dataStream.print(cfg.eot_ch);
  }

  // Verbose timeout error
#ifdef DEBUG_GPIBbus_RECEIVE
  if (r > 0) {
    DB_PRINT(F("Timeout waiting for sender!"),"");
    DB_PRINT(F("Timeout waiting for transfer to complete!"),"");
  }
#endif

  // Return controller to idle state
  if (cfg.cmode == 2) {


    // Untalk bus and unlisten controller
    if (unAddressDevice()) {
#ifdef DEBUG_GPIBbus_RECEIVE
      DB_PRINT(F("Failed to untalk bus"),"");
#endif
    }

    // Set controller back to idle state
    setControls(CIDS);

  } else {
    // Set device back to idle state
    setControls(DIDS);
  }

  // Reset break flag
  if (txBreak) txBreak = false;

#ifdef DEBUG_GPIBbus_RECEIVE
  DB_PRINT(F("done."),"");
#endif

  if (r > 0) return ERR;

  return OK;

}


/***** Send a series of characters as data to the GPIB bus *****/
void GPIBbus::sendData(char *data, uint8_t dsize) {

  bool err = false;

  // Set control pins for writing data (ATN unasserted)
  if (cfg.cmode == 2) {
    setControls(CTAS);
  } else {
    setControls(DTAS);
  }

#ifdef DEBUG_GPIBbus_SEND
  DB_PRINT(F("write data mode is set."),"");
  DB_PRINT(F("begin send loop->"),"");
#endif

  // Write the data string
  for (int i = 0; i < dsize; i++) {
    // If EOI asserting is on
    if (cfg.eoi) {
      // Send all characters
      err = writeByte(data[i], NO_EOI);
    } else {
      // Otherwise ignore non-escaped CR, LF and ESC
      if ((data[i] != CR) && (data[i] != LF) && (data[i] != ESC)) err = writeByte(data[i], NO_EOI);
    }

#ifdef DEBUG_GPIBbus_SEND
    DB_RAW_PRINT(data[i]);
#endif

    if (err) break;
  }

#ifdef DEBUG_GPIBbus_SEND
  DB_PRINT(F("<-End of send loop."),"");
#endif

//  if (!err  && !dataContinuity) {
  if (!err) {
    // Write terminators according to EOS setting
    // Do we need to write a CR?
    if ((cfg.eos & 0x2) == 0) {
      writeByte(CR, NO_EOI);
#ifdef DEBUG_GPIBbus_SEND
      DB_PRINT(F("appended CR"),"");
#endif
    }
    // Do we need to write an LF?
    if ((cfg.eos & 0x1) == 0) {
      writeByte(LF, NO_EOI);
#ifdef DEBUG_GPIBbus_SEND
      DB_PRINT(F("appended LF"),"");
#endif
    }
  }

  // If EOI enabled and no more data to follow then assert EOI
//  if (cfg.eoi && !dataContinuity) {
  if (cfg.eoi) {
    outputCtrl(EOI_BIT);
    assertCtrl(EOI_BIT);
    delayMicroseconds(40);
    clearCtrl(EOI_BIT);
#ifdef DEBUG_GPIBbus_SEND
    DB_PRINT(F("Asserted EOI"),"");
#endif
  }

  if (cfg.cmode == 2) {   // Controller mode
/*
    if (!err) {
      if (!addressingSuppressed) {
        // Untalk controller and unlisten bus
        if (unAddressDevice()) {
#ifdef DEBUG_GPIBbus_SEND
          DB_PRINT(F("failed to unlisten bus"),"");
#endif
        }

#ifdef DEBUG_GPIBbus_SEND
        DB_PRINT(F("unlisten done"),"");
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
    DB_PRINT(F("done."),"");
#endif
}



/**************************************************/
/***** FUCTIONS TO READ/WRITE DATA TO STORAGE *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

#ifdef EN_STORAGE


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
      // if SN7516X and ATN are correct, then SRQ, NRFD, NDAC should be outputs and EOI, DAV should be inputs
      inputCtrl(SRQ_BIT | NRFD_BIT | NDAC_BIT | IFC_BIT);
      outputCtrl(ATN_BIT | REN_BIT | EOI_BIT | DAV_BIT);
      // Set pin state
      clearCtrl(ATN_BIT | EOI_BIT | DAV_BIT);
      assertCtrl(REN_BIT);
      // setGpibState(0b11011111, 0b11111111, 0); WHUT? SETS STATE ON INPUT BITS
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
      DB_PRINT(F("Initialised GPIB control mode"),"");
#endif
      break;

    case CIDS:  // Controller idle state
      // if SN7516X_TE (DC is low in Controller mode) and ATN are correct, then REN, IFC, NDAC and NRFD should be outputs and SRQ, EOI and DAV should be input
      inputCtrl(SRQ_BIT | NRFD_BIT | NDAC_BIT);
      outputCtrl(ATN_BIT | EOI_BIT | DAV_BIT);
      // setGpibState(0b10111000, 0b10011110, 1); WHUT? TRIES TO SET DIRECTION EXCLUDED BY MASK
      clearCtrl(ATN_BIT | EOI_BIT | DAV_BIT);
      // setGpibState(0b11011111, 0b10011110, 0); WHUT? TRIES TO SET BITS NOT IN MASK, SET STATE ON INPUT BITS
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines to idle state"),"");
#endif
      break;

    case CCMS:  // Controller active - send commands
      // if SN75161_TE is correct, REN should be an output, SRQ should be an input
      inputCtrl(NRFD_BIT | NDAC_BIT);
      outputCtrl(ATN_BIT | EOI_BIT | DAV_BIT | IFC_BIT);
      // setGpibState(0b10111001, 0b10011111, 1); WHUT? TRIES TO SET REN_BIT NOT IN MASK
      assertCtrl(ATN_BIT);
      clearCtrl(EOI_BIT | DAV_BIT | NRFD_BIT | NDAC_BIT | IFC_BIT);
      // setGpibState(0b01011111, 0b10011111, 0); WHUT? TRIED TO CLEAR SRQ_BIT NOT IN MASK; SET STATE ON INPUT BITS NRFD and NDAC
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines for sending a command"),"");
#endif
      break;

    case CLAS:  // Controller - read data bus
      // Set state for receiving data
      // if SN7516X_TE and ATN are correct, then REN and IFC are outputs, and SRQ is an input
      inputCtrl(EOI_BIT | DAV_BIT);
      outputCtrl(ATN_BIT | NRFD_BIT | NDAC_BIT);
      // setGpibState(0b10100110, 0b10011110, 1); WHUT? TRIES TO SET DIRECTION ON REN_BIT NOT IN MASK
      assertCtrl(EOI_BIT | DAV_BIT);
      clearCtrl(ATN_BIT | NRFD_BIT | NDAC_BIT);
      // setGpibState(0b11011000, 0b10011110, 0); WHUT? TRIES TO CLEAR SRQ_BIT NOT IN MASK; SET STATE ON INPUT BITS EOI and DAV
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines for reading data"),"");
#endif
      break;

    case CTAS:  // Controller - write data bus
      // if SN7516X_TE is correct, then REN and IFC are outputs, SRQ is an input
      inputCtrl(NRFD_BIT | NDAC_BIT);
      outputCtrl(ATN_BIT | EOI_BIT | DAV_BIT);
      // setGpibState(0b10111001, 0b10011110, 1); WHUT? TRIES TO SET DIRECTION ON REN_BIT AND IFC_BIT NOT IN MASK
      clearCtrl(ATN_BIT | EOI_BIT | DAV_BIT | NRFD_BIT | NDAC_BIT);
      // setGpibState(0b11011111, 0b10011110, 0); WHUT? TRIES TO CLEAR SRQ_BIT AND IFC_BIT NOT IN MASK
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines for writing data"),"");
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
      // if SN7516X_TE and _DC are high, SRQ and DAV should be outputs, ATN is high so EOI should also be output
      inputCtrl(ALL_BITS);
      // setGpibState(0b00000000, 0b11111111, 1);
      clearCtrl(ALL_BITS);
      // setGpibState(0b11111111, 0b11111111, 0); WHUT? TRIES TO SET STATE ON ALL INPUTS!
      // Set data bus to idle state
      readyGpibDbus();
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Initialised GPIB listener mode"),"");
#endif
      break;

    case DIDS:  // Device idle state
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif
      // if SN7516X_TE and _DC are high, SRQ and DAV should be outputs, ATN is high so EOI should also be output 
      inputCtrl(DAV_BIT | NRFD_BIT | NDAC_BIT);
      // setGpibState(0b00000000, 0b00001110, 1);
      clearCtrl(DAV_BIT | NRFD_BIT | NDAC_BIT);
      // setGpibState(0b11111111, 0b00001110, 0); WHUT? TRIES TO SET STATE ON BITS NOT IN MASK
      // Set data bus to idle state
      readyGpibDbus();
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines to idle state"),"");
#endif
      break;

    case DLAS:  // Device listner active (actively listening - can handshake)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif
      // if SN7616X_TE is low and _DC is high, then SRQ should be output
      inputCtrl(EOI_BIT | DAV_BIT);
      outputCtrl(NRFD_BIT | NDAC_BIT);
      // setGpibState(0b00000110, 0b00011110, 1);
      assertCtrl(NRFD_BIT | NDAC_BIT);
      clearCtrl(EOI_BIT | DAV_BIT);
      // setGpibState(0b11111001, 0b00011110, 0); WHUT? TRIES TO SET STATE ON BITS NOT IN MASK, SET STATE ON INPUT BITS EOI AND DAV
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines to idle state"),"");
#endif
      break;

    case DTAS:  // Device talker active (sending data)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif
      // if SN7516X_TE and _DC are high, SRQ should be an output, and ATN, REN, IFC should be inputs
      inputCtrl(NRFD_BIT | NDAC_BIT);
      outputCtrl(EOI_BIT | DAV_BIT);
      // setGpibState(0b00011000, 0b00011110, 1);
      assertCtrl(NRFD_BIT | NDAC_BIT);
      clearCtrl(EOI_BIT | DAV_BIT);
      // setGpibState(0b11111001, 0b00011110, 0); WHUT? TRIES TO SET STATE ON BITS NOT IN MASK, SET STATE ON INPUT BITS NRFD AND NDAC
#ifdef DEBUG_GPIBbus_CONTROL
      DB_PRINT(F("Set GPIB lines for listening as addresed device"),"");
#endif
      break;
#ifdef DEBUG_GPIBbus_CONTROL
    default:
      // Should never get here!
      //      setGpibState(0b00000110, 0b10111001, 0b11111111);
      DB_PRINT(F("Unknown GPIB state requested!"),"");
#endif
  }

  // Save state
  cstate = state;

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
#ifdef DEBUG_GPIBbus_DEVICE
  DB_PRINT(F("done."),"");
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
#ifdef DEBUG_GPIBbus_DEVICE
  DB_PRINT(F("addressDevice: "),addr);
#endif
  if (talk) {
    // Device to talk, controller to listen
    if (sendCmd(GC_TAD + addr)) return ERR;
  } else {
    // Device to listen, controller to talk
    if (sendCmd(GC_LAD + addr)) return ERR;
  }

  // Set flag
  deviceAddressed = true;
  return OK;
}


/***** Returns status of controller device addressing *****/
/*
 * true = device addressed; false = device is not addressed
 */
bool GPIBbus::haveAddressedDevice(){
  return deviceAddressed;
}


/***** Device is addressed to listen? *****/
bool GPIBbus::isDeviceAddressedToListen(){
//  if (deviceAddressedState == DLAS) return true;
  if (cstate == DLAS) return true;
  return false;
}


/***** Device is addressed to talk? *****/
bool GPIBbus::isDeviceAddressedToTalk(){
//  if (deviceAddressedState == DTAS) return true;
  if (cstate == DTAS) return true;
  return false;
}


/***** Device is not addressed? *****/
bool GPIBbus::isDeviceInIdleState(){
  if (cstate == DIDS) return true;
  return false;
}


/***** Clear the data bus - set to listen state *****/
void GPIBbus::clearDataBus(){
  readyGpibDbus();
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

  bool atnStat = isAsserted(ATN); // Capture state of ATN
  *eoi = false;

  // Wait for interval to expire
  while ( (unsigned long)(currentMillis - startMillis) < timeval ) {

    if (cfg.cmode == 1) {
      // If IFC has been asserted then abort
      if (isAsserted(IFC)) {
#ifdef DEBUG_GPIBbus_RECEIVE
        DB_PRINT(F("IFC detected]"),"");
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
      clearCtrl(NRFD_BIT);
      // setGpibState(0b00000100, 0b00000100, 0);
      stage = 6;
    }

    if (stage == 6) {
      // Wait for DAV to go LOW indicating talker has finished setting data lines..
//      if (digitalRead(DAV) == LOW) {
      if (getGpibPinState(DAV) == LOW) {
        // Assert NRFD (Busy reading data)
	assertCtrl(NRFD_BIT);
        // setGpibState(0b00000000, 0b00000100, 0);
        stage = 7;
      }
    }

    if (stage == 7) {
      // Check for EOI signal
      if (readWithEoi && isAsserted(EOI)) *eoi = true;
      // read from DIO
      *db = readGpibDbus();
      // Unassert NDAC signalling data accepted
      clearCtrl(NDAC_BIT);
      // setGpibState(0b00000010, 0b00000010, 0);
      stage = 8;
    }

    if (stage == 8) {
      // Wait for DAV to go HIGH indicating data no longer valid (i.e. transfer complete)
//      if (digitalRead(DAV) == HIGH) {
      if (getGpibPinState(DAV) == HIGH) {
        // Re-assert NDAC - handshake complete, ready to accept data again
	assertCtrl(NDAC_BIT);
        // setGpibState(0b00000000, 0b00000010, 0);
        stage = 9;
        break;
      }
    }

    // Increment time
    currentMillis = millis();

  }

  // Completed
  if (stage == 9) return 0;

//  if (stage==1) return 4;
//  if (stage==2) return 3;

  // Otherwise return stage
#ifdef DEBUG_GPIBbus_RECEIVE
  if ( (stage==4) || (stage==8) ) {
    DB_PRINT(F("DAV timout!"),"");
  }else{
    DB_PRINT(F("Error: "), stage);
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
        DB_PRINT(F("IFC detected!"),"");
#endif
        stage = 1;
        break;
      }

      // If ATN has been asserted we need to abort and listen
      if (isAsserted(ATN)) {
        setControls(DLAS);
#ifdef DEBUG_GPIBbus_SEND
        DB_PRINT(F("ATN detected!"),"");
#endif
        stage = 2;
        break;
      }
    }

    // Wait for NDAC to go LOW (indicating that devices (stage==4) || (stage==8) ) are at attention)
    if (stage == 4) {
//      if (digitalRead(NDAC) == LOW) stage = 5;
      if (getGpibPinState(NDAC) == LOW) stage = 5;
    }

    // Wait for NRFD to go HIGH (indicating that receiver is ready)
    if (stage == 5) {
//      if (digitalRead(NRFD) == HIGH) stage = 6;
      if (getGpibPinState(NRFD) == HIGH) stage = 6;
    }

    if (stage == 6){
      // Place data on the bus
      setGpibDbus(db);
      if (cfg.eoi && isLastByte) {
        // If EOI enabled and this is the last byte then assert DAV and EOI
#ifdef DEBUG_GPIBbus_SEND
        DB_PRINT(F("Asserting EOI..."),"");
#endif
	assertCtrl(EOI_BIT | DAV_BIT);
        // setGpibState(0b00000000, 0b00011000, 0);
      }else{
        // Assert DAV (data is valid - ready to collect)
	assertCtrl(DAV_BIT);
        // setGpibState(0b00000000, 0b00001000, 0);
      }
      stage = 7;
    }

    if (stage == 7) {
      // Wait for NRFD to go LOW (receiver accepting data)
//      if (digitalRead(NRFD) == LOW) stage = 8;
      if (getGpibPinState(NRFD) == LOW) stage = 8;
    }

    if (stage == 8) {
      // Wait for NDAC to go HIGH (data accepted)
//      if (digitalRead(NDAC) == HIGH) {
      if (getGpibPinState(NDAC) == HIGH) {
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
      if (cfg.eoi && isLastByte)
	clearCtrl(EOI_BIT | DAV_BIT);
        // setGpibState(0b00011000, 0b00011000, 0);
    }else{
      // Unassert DAV
      clearCtrl(DAV_BIT);
      // setGpibState(0b00001000, 0b00001000, 0);
    }
    // Reset the data bus
    setGpibDbus(0);
    return 0;
  }

  // Otherwise timeout or ATN/IFC return stage at which it ocurred
#ifdef DEBUG_GPIBbus_SEND
  switch (stage) {
    case 4:
      DB_PRINT(F("NDAC timeout!"),"");
      break;
    case 5:
      DB_PRINT(F("NRFD timout!"),"");
      break;
    case 7:
      DB_PRINT(F("NRFD timout!"),"");
      break;
    case 8:
      DB_PRINT(F("NDAC timout!"),"");
      break;
    default:
      DB_PRINT(F("Error: "), stage);
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
