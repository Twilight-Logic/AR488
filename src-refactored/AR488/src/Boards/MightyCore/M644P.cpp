#include "M644P.h"


/***************************************************/
/***** PANDUINO/MIGHTYCORE MCGRAW BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA644P_MCGRAW
/*
  Data pin map
  ------------
  DIO1_PIN  10 : GPIB 1  : PD2
  DIO2_PIN  11 : GPIB 2  : PD3
  DIO3_PIN  12 : GPIB 3  : PD4
  DIO4_PIN  13 : GPIB 4  : PD5
  DIO5_PIN  14 : GPIB 13 : PD6
  DIO6_PIN  15 : GPIB 14 : PD7
  DIO7_PIN  16 : GPIB 15 : PC0
  DIO8_PIN  17 : GPIB 16 : PC1

  Control pin map
  ---------------
  IFC_PIN   22 : GPIB  9 : PC6 : b0
  NDAC_PIN  21 : GPIB  8 : PC5 : b1
  NRFD_PIN  20 : GPIB  7 : PC4 : b2
  DAV_PIN   19 : GPIB  6 : PC3 : b3
  EOI_PIN   18 : GPIB  5 : PC2 : b4
  REN_PIN   23 : GPIB 17 : PA0 : b5
  SRQ_PIN   24 : GPIB 10 : PC7 : b6
  ATN_PIN   31 : GPIB 11 : PA7 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRD |= 0b11111100;
    DDRC |= 0b00000011;
    PORTD |= 0b11111100; // PORTC bits 7,5,3,1 high
    PORTC |= 0b00000011; // PORTA bits 6,4,2,0 high
    return;
  }

  // Set data pins to input
  DDRD &= 0b00000011;  
  DDRC &= 0b11111100;  
  
  PORTD |= 0b11111100; // PORTD bits 7, 6, 5, 4, 3, 2 input_pullup
  PORTC |= 0b00000011; // PORTC bits 1, 0 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~((PIND >> 2 & 0b00111111) + (PINC << 6 & 0b11000000));
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
  DDRD |= 0b11111100;
  DDRC |= 0b00000011;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTC = (PORTC & ~0b00000011) | ((db & 0b11000000) >> 6);
  PORTD = (PORTD & ~0b11111100) | ((db & 0b00111111) << 2);
}


/***** Reverse the order of the bits in a byte *****/
uint8_t reverseBits(uint8_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  // PORT A - use bits 5 and 7. Map to port A bits 0 and 7
  uint8_t portAb = ((bits & 0x20) >> 5) + (bits &  0x80);
  uint8_t portAm = ((mask & 0x20) >> 5) + (mask &  0x80);

  // PORT C- use the 5 right-most bits (bits 0 - 4) and bit 6
  // Reverse bits 0-4 and map to bits 2-6. Map bit 6 to bit 7
  uint8_t portCb = (reverseBits(bits & 0x1F) >> 1) + ((bits & 0x40) << 1);
  uint8_t portCm = (reverseBits(mask & 0x1F) >> 1) + ((mask & 0x40) << 1);

  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  // Set pin states using mask
  PORTA = ( (PORTA & ~portAm) | (portAb & portAm) );
  PORTC = ( (PORTC & ~portCm) | (portCb & portCm) );

}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  // PORT A - use bits 5 and 7. Map to port A bits 0 and 7
  uint8_t portAb = ((bits & 0x20) >> 5) + (bits &  0x80);
  uint8_t portAm = ((mask & 0x20) >> 5) + (mask &  0x80);

  // PORT C- use the 5 right-most bits (bits 0 - 4) and bit 6
  // Reverse bits 0-4 and map to bits 2-6. Map bit 6 to bit 7
  uint8_t portCb = (reverseBits(bits & 0x1F) >> 1) + ((bits & 0x40) << 1);
  uint8_t portCm = (reverseBits(mask & 0x1F) >> 1) + ((mask & 0x40) << 1);

  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  // Set pin direction registers using mask
  DDRA = ( (DDRA & ~portAm) | (portAb & portAm) );
  DDRC = ( (DDRC & ~portCm) | (portCb & portCm) );

}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}


#endif // AR488_MEGA644P_MCGRAW
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PANDUINO/MIGHTYCORE MCGRAW BOARD LAYOUT *****/
/***************************************************/

