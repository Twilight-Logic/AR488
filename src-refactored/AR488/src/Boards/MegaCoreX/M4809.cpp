#include "M4809.h"

/**************************************/
/***** KOFEN_POE_ETHERNET_ADAPTOR *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#if defined(KOFEN_POE_ETHERNET_ADAPTOR)


/***** Control pin map *****/
/*
  Data pin map
  ------------
  DIO1_PIN  22 : GPIB 1  : PD0
  DIO2_PIN  23 : GPIB 2  : PD1
  DIO3_PIN  24 : GPIB 3  : PD2
  DIO4_PIN  25 : GPIB 4  : PD3
  DIO5_PIN  26 : GPIB 13 : PD4
  DIO6_PIN  27 : GPIB 14 : PD5
  DIO7_PIN  28 : GPIB 15 : PD6
  DIO8_PIN  29 : GPIB 16 : PD7

  Control pin map
  ---------------
  IFC_PIN   18 : GPIB  9 : PC4 : b0
  NDAC_PIN  17 : GPIB  8 : PC3 : b1
  NRFD_PIN  16 : GPIB  7 : PC2 : b2
  DAV_PIN   15 : GPIB  6 : PC1 : b3
  EOI_PIN   14 : GPIB  5 : PC0 : b4
  REN_PIN   21 : GPIB 17 : PC7 : b5
  SRQ_PIN   19 : GPIB 10 : PC5 : b6
  ATN_PIN   20 : GPIB 11 : PC6 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=LOW, 1=HIGH
    mask : 0=unaffected, 1=affected
*/



uint8_t readPortPullupReg(PORT_t& port){
  uint8_t reg = 0;
  reg |= (port.PIN0CTRL & PORT_PULLUPEN_bm) >> 3;
  reg |= (port.PIN1CTRL & PORT_PULLUPEN_bm) >> 2;
  reg |= (port.PIN2CTRL & PORT_PULLUPEN_bm) >> 1;
  reg |= (port.PIN3CTRL & PORT_PULLUPEN_bm);
  reg |= (port.PIN4CTRL & PORT_PULLUPEN_bm) << 1;
  reg |= (port.PIN5CTRL & PORT_PULLUPEN_bm) << 2;
  reg |= (port.PIN6CTRL & PORT_PULLUPEN_bm) << 3;
  reg |= (port.PIN7CTRL & PORT_PULLUPEN_bm) << 4;
  return reg;
}


/*
uint8_t readPortPullupReg(PORT_t port){
  const uint8_t base = 0x10;
  uint8_t reg = 0;
  PORT_t * offset = &port + base;
  for (uint8_t i=0; i<8; i++) {
    offset += i;
//    reg |= (*offset & PORT_PULLUPEN_bm);
  }
  return reg;
}
*/


void setPortPullupBits(PORT_t& port, uint8_t reg){
  port.PIN0CTRL |= ((reg<<3) & PORT_PULLUPEN_bm);
  port.PIN1CTRL |= ((reg<<2) & PORT_PULLUPEN_bm);
  port.PIN2CTRL |= ((reg<<1) & PORT_PULLUPEN_bm);
  port.PIN3CTRL |= (reg & PORT_PULLUPEN_bm);
  port.PIN4CTRL |= ((reg>>1) & PORT_PULLUPEN_bm);
  port.PIN5CTRL |= ((reg>>2) & PORT_PULLUPEN_bm);
  port.PIN6CTRL |= ((reg>>3) & PORT_PULLUPEN_bm);
  port.PIN7CTRL |= ((reg>>4) & PORT_PULLUPEN_bm);  
}


// PORT_t port + offset (0x10..17) is PORT.PINnCTRL (10 = 0, 17=7)

/*
void setPortPullupBits(PORT_t port, uint8_t reg){
  const uint8_t offset = 0x10;
  PORT_t * ptr = &port + offset;
  for (uint8_t i=0; i<8; i++) {
    if ((reg>>i) & 1) *ptr[i] = PORT_PULLUPEN_bm;
  }
}
*/

/***** Set the GPIB data bus to input pullup *****/

void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    PORTD.DIR &= 0b11111111;
    // Set PORTD bits to high
    PORTD.OUT = 0b11111111;
    return;
  }

  // Set data pins to input
  PORTD.DIR &= 0b00000000;
  // Set PORTD bits to input_pullup
  setPortPullupBits(PORTD, 0b11111111);
}


/***** Read the GPIB data bus wires to collect the byte of data *****/

uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~PORTD.IN;
}



/***** Set the GPIB data bus to output and with the requested byte *****/

void setGpibDbus(uint8_t db) {

  // Set data pins as outputs
  PORTD.DIR |= 0b11111111;

  // Set data bus (GPIB states are inverted)
  PORTD.OUT = ~db;
}



/***** Reverse the order of the bits in a byte *****/
uint8_t reverseBits(uint8_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}


uint8_t bitsToPort(uint8_t bits){
  // PORT C - keep bits 0-4, rotate bit 5 right 3 positions, bit 6 & 7 left 1 position on register
  return (reverseBits(bits & 0x1F) >> 3) | ((bits & 0x20) << 2) | ((bits & 0x40) >> 1) | ((bits & 0x80) >> 1);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  if (!mask) return;  // Empty mask does nothing!
  uint8_t portCb = bitsToPort(bits);
  uint8_t portCm = bitsToPort(mask);  

  // Set pin states using mask
  PORTC.OUT = ( (PORTC.OUT & ~portCm) | (portCb & portCm) );
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  uint8_t portCb = bitsToPort(bits);
  uint8_t portCm = bitsToPort(mask);  

  // Set pin direction registers using mask
  PORTC.DIR = ( (PORTC.DIR & ~portCm) | (portCb & portCm) );

  // Set inputs to input_pullup, outputs  to output
  uint8_t reg = (readPortPullupReg(PORTC) & ~portCm);
  uint8_t toset = (~portCb & portCm);
  reg |= toset;
  setPortPullupBits(PORTC, reg);
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}




/***** Set the GPIB data bus to input pullup *****/
/*
void readyGpibDbus() {
  // Set all PORTD pins (DIO1–DIO8) to input with pull-up resistors
  PORTD.DIRCLR = 0xFF;          // Set PORTD pins 0–7 as input
  PORTD.PIN0CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 0
  PORTD.PIN1CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 1
  PORTD.PIN2CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 2
  PORTD.PIN3CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 3
  PORTD.PIN4CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 4
  PORTD.PIN5CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 5
  PORTD.PIN6CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 6
  PORTD.PIN7CTRL = PORT_PULLUPEN_bm; // Enable pull-up on PORTD pin 7
}
*/
/*
void readyGpibDbus() {
  // Set all PORTD pins (DIO1–DIO8) to input with pull-up resistors
  PORTD.DIRCLR = 0xFF;
  setPortPullupBits(PORTD, 0xFF);
}
*/
/***** Read the GPIB data bus wires to collect the byte of data *****/
/*
uint8_t readGpibDbus() {
  // Read the entire PORTD directly and invert for GPIB logic levels
  return ~PORTD.IN;
}
*/

/***** Set the GPIB data bus to output and with the requested byte *****/
/*
void setGpibDbus(uint8_t db) {
  // Set PORTD pins (DIO1–DIO8) as outputs
  PORTD.DIRSET = 0xFF;

  // Write the inverted byte to PORTD to match GPIB logic levels
  PORTD.OUT = ~db;
}
*/

/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
   state: 0=LOW; 1=HIGH/INPUT_PULLUP
   dir  : 0=input; 1=output;
   mode:  0=set pin state; 1=set pin direction
*/
/*
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {
  uint8_t portCb = 0; // Output value for PORTC
  uint8_t portCm = 0; // Mask for PORTC

  // Map GPIB control bits to PORTC pins
  portCb |= (bits & (1 << 0)) << 4; // IFC_PIN  (bit 0 -> PC4)
  portCb |= (bits & (1 << 1)) << 2; // NDAC_PIN (bit 1 -> PC3)
  portCb |= (bits & (1 << 2)) << 0; // NRFD_PIN (bit 2 -> PC2)
  portCb |= (bits & (1 << 3)) >> 2; // DAV_PIN  (bit 3 -> PC1)
  portCb |= (bits & (1 << 4)) >> 4; // EOI_PIN  (bit 4 -> PC0)
  portCb |= (bits & (1 << 5)) << 2; // REN_PIN  (bit 5 -> PC7)
  portCb |= (bits & (1 << 6)) >> 1; // SRQ_PIN  (bit 6 -> PC5)
  portCb |= (bits & (1 << 7)) >> 1; // ATN_PIN  (bit 7 -> PC6)

  portCm |= (mask & (1 << 0)) << 4; // IFC_PIN  (bit 0 -> PC4)
  portCm |= (mask & (1 << 1)) << 2; // NDAC_PIN (bit 1 -> PC3)
  portCm |= (mask & (1 << 2)) << 0; // NRFD_PIN (bit 2 -> PC2)
  portCm |= (mask & (1 << 3)) >> 2; // DAV_PIN  (bit 3 -> PC1)
  portCm |= (mask & (1 << 4)) >> 4; // EOI_PIN  (bit 4 -> PC0)
  portCm |= (mask & (1 << 5)) << 2; // REN_PIN  (bit 5 -> PC7)
  portCm |= (mask & (1 << 6)) >> 1; // SRQ_PIN  (bit 6 -> PC5)
  portCm |= (mask & (1 << 7)) >> 1; // ATN_PIN  (bit 7 -> PC6)

  switch (mode) {
    case 0: // Set pin states
      PORTC.OUT = (PORTC.OUT & ~portCm) | (portCb & portCm);
      break;
    case 1: // Set pin directions
      PORTC.DIR = (PORTC.DIR & ~portCm) | (portCb & portCm);
      break;
  }
}

void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  setGpibState(bits, mask, 1);
};

void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  setGpibState(bits, mask, 0);
};


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}
*/

#endif  // ESP VARIANTS
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** KOFEN_POE_ETHERNET_ADAPTOR *****/
/**************************************/
