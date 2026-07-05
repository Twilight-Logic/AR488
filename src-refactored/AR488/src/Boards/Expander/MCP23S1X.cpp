#include "MCP23S1X.h"

#ifdef MCP23S1X_EXPANDER


/**************************************/
/***** MCP23S17/8 EXPANDER LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvv *****/

/*
  Data pin map
  ------------
  DIO1_PIN   0 : GPIB 1  : B0
  DIO2_PIN   1 : GPIB 2  : B1
  DIO3_PIN   2 : GPIB 3  : B2
  DIO4_PIN   3 : GPIB 4  : B3
  DIO5_PIN   4 : GPIB 13 : B4
  DIO6_PIN   5 : GPIB 14 : B5
  DIO7_PIN   6 : GPIB 15 : B6
  DIO8_PIN   7 : GPIB 16 : B7

  Control pin map
  ---------------
  IFC_PIN    0 : GPIB  9 : PA0 : b0
  NDAC_PIN   1 : GPIB  8 : PA1 : b1
  NRFD_PIN   2 : GPIB  7 : PA2 : b2
  DAV_PIN    3 : GPIB  6 : PA3 : b3
  EOI_PIN    4 : GPIB  5 : PA4 : b4
  REN_PIN    5 : GPIB 17 : PA5 : b5
  SRQ_PIN    6 : GPIB 10 : PA6 : b6
  ATN_PIN    7 : GPIB 11 : PA7 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


// MCP23S17 hardware config
const uint8_t chipSelect = MCP_SELECTPIN;
const uint8_t mcpAddr = MCP_ADDRESS;      // Must be between 0 and 7
uint8_t mcpIntAReg = 0;


/***** Ready the SPI bus *****/
void mcpInit(){
  SPI.begin();
  // Optional: Clock divider (slow down the bus speed [optional])
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  // Set expander configuration register
  // (Bit 1=0 sets active low for Int A)
  // (Bit 3=1 enables hardware address pins (MCP23S17 only)
  // (Bit 7=0 sets registers to be in same bank)
  mcpByteWrite(MCPCON, 0b00001000);
  // Enable MCP23S17 interrupts
  mcpInterruptsEn();
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (stat == OUTPUT) {
    mcpByteWrite(MCPDIRB, 0b00000000);  // Port direction: 0 = output; 1 = input;
    // Set output pins HIGH
    mcpByteWrite(MCPPORTB, 0b11111111);
  }

  // Set data pins to input
  mcpByteWrite(MCPDIRB, 0b11111111);  // Port direction: 0 = output; 1 = input;
  mcpByteWrite(MCPPUB, 0b11111111);   // 1 = Pullup resistors enabled
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~mcpByteRead(MCPPORTB);
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
  mcpByteWrite(MCPDIRB, 0b00000000);  // Port direction: 0 = output; 1 = input;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  mcpByteWrite(MCPPORTB, db);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  uint8_t portAb = bits;
  uint8_t portAm = mask;

  uint8_t regByte = 0;
  uint8_t regMod = 0; 


  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  // Set pin states using mask
  regByte = mcpByteRead(MCPPORTA);
  regMod = (regByte & ~portAm) | (portAb & portAm);
  mcpByteWrite(MCPPORTA, regMod);
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  uint8_t portAb = bits;
  uint8_t portAm = mask;

  uint8_t regByte = 0;
  uint8_t regMod = 0; 


  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  // Set pin direction registers using mask
  regByte = ~mcpByteRead(MCPDIRA);   // Note: on MCP23S17 0 = output, 1 = input
  regMod = (regByte & ~portAm) | (portAb & portAm);
  mcpByteWrite(MCPDIRA, ~regMod);    // Note: on MCP23S17 0 = output, 1 = input

}



/***** MCP23S1X interrupt handler *****/
/*
 * Interrput pin on Arduino configure with attachInterrupt
 */
void mcpIntHandler() {
//  mcpIntA = true;
//  Serial.println(F("MCP Interrupt triggered"));
  mcpIntAReg = mcpByteRead(MCPINTCAPA);
}


uint8_t getMcpIntAReg(){
  return mcpIntAReg;
}


/***** Read from the MCP23S1X *****/
/*
 * reg : register we want to read , e.g. MCPPORTA or MCPPORTB
 */
uint8_t mcpByteRead(uint8_t reg){
  uint8_t db;
  digitalWrite(chipSelect, LOW);            // Enable MCP communication
  SPI.transfer(MCPREAD | (mcpAddr << 1));   // Write opcode + chip address + write bit
  SPI.transfer(reg);                        // Write the register we want to read
  db = SPI.transfer(0x00);                  // Send any byte. Function returns low byte (port A value) which is ignored
  digitalWrite(chipSelect, HIGH);           // Enable MCP communication
  return db;
}


/***** Write to the MCP23S1X *****/
void mcpByteWrite(uint8_t reg, uint8_t db){
  digitalWrite(chipSelect, LOW);            // Enable MCP communication
  SPI.transfer(MCPWRITE | (mcpAddr << 1));  // Write opcode (with write bit set) + chip address
  SPI.transfer(reg);                        // Write register we want to change
  SPI.transfer(db);                         // Write data byte
  digitalWrite(chipSelect, HIGH);           // Stop MCP communication
}


/***** Read status of control port pins *****/
/*
 * Pin value between 0 and 7
 * Control bus = port A)
 */
uint8_t mcpDigitalRead(uint8_t pin) {
  // If the pin value is larger than 7 then do nothing and return
  // Zero or larger value is implied by the variable type
  if (pin > 7) return 0x0;
  // Read the port A pin state, extract and return HIGH/LOW state for the requested pin
  return mcpByteRead(MCPPORTA) & (1 << pin) ? HIGH : LOW;
}


/***** Get the status of an MCP23S17 pin) *****/
uint8_t getGpibPinState(uint8_t pin){
  return mcpDigitalRead(pin);
}


/***** Configure pins that will generate an interrupt *****/
void mcpInterruptsEn(){
  // Set to interrupt mode for compare to previous
  mcpByteWrite(MCPINTCONA, 0b00000000);
  // Enable interrupt to detect pin state change on pins 4, 6 and 7 (EOI_PIN, SRQ_PIN and ATN_PIN)
  mcpByteWrite(MCPINTENA, 0b11010000);
}

#endif  // #ifdef MCP23S1X_EXPANDER
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MCP23S17/8 EXPANDER LAYOUT *****/
/**************************************/

