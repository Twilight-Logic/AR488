#include "M32u4.h"


/***********************************************************/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_32U4_MICRO
/*
  Data pin map
  ------------
  DIO1_PIN   3 : GPIB 1  : PD0
  DIO2_PIN  15 : GPIB 2  : PB1
  DIO3_PIN  16 : GPIB 3  : PB2
  DIO4_PIN  14 : GPIB 4  : PB3
  DIO5_PIN   8 : GPIB 13 : PB4
  DIO6_PIN   9 : GPIB 14 : PB5
  DIO7_PIN  10 : GPIB 15 : PB6
  DIO8_PIN   6 : GPIB 16 : PD7

  Control pin map
  ---------------
  IFC_PIN    4 : GPIB  9 : PD4 : b0
  NDAC_PIN  A3 : GPIB  8 : PF4 : b1
  NRFD_PIN  A2 : GPIB  7 : PF5 : b2
  DAV_PIN   A1 : GPIB  6 : PF6 : b3
  EOI_PIN   A0 : GPIB  5 : PF7 : b4
  REN_PIN    7 : GPIB 17 : PC6 : b5
  SRQ_PIN    5 : GPIB 10 : PE6 : b6
  ATN_PIN    2 : GPIB 11 : PD1 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRB |= 0b01111110;
    DDRD |= 0b10000001;
    PORTB |= 0b01111110; // PORTC bits 7,5,3,1 high
    PORTD |= 0b10000001; // PORTA bits 6,4,2,0 high
    return;
  }

  // Set data pins to input
  DDRB  &= 0b10000001 ;
  DDRD  &= 0b01111110 ;
  PORTB |= 0b01111110; // PORTB bits 6,5,4,3,2,1 input_pullup
  PORTD |= 0b10000001; // PORTD bits 7,0 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  return ~((PIND & 0b10000001) | (PINB & 0b01111110)) ;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  // Set data pins as outputs
  DDRB |= 0b01111110;
  DDRD |= 0b10000001;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTB = (PORTB & ~0b01111110) | (db & 0b01111110) ;
  PORTD = (PORTD & ~0b10000001) | (db & 0b10000001);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  // most of the time, only these bits change

  if (mask & 0b00011110) {

    // PORTF - NDAC_PIN, NRFD_PIN, DAV_PIN and EOI_PIN bits 1-4 rotated into bits 4-7
    uint8_t portFb = (bits & 0x1e) << 3;
    uint8_t portFm = (mask & 0x1e) << 3;

    // Set pin states using mask
    PORTF = ( (PORTF & ~portFm) | (portFb & portFm) );

  }

  if (mask & 0b11100001) {

    // PORTC - REN_PIN bit 5 rotated into bit 6
    uint8_t portCb = (bits & 0x20) << 1;
    uint8_t portCm = (mask & 0x20) << 1;
    // PORTD - IFC_PIN bit 0 rotated into bit 4 and ATN_PIN bit 7 rotated into 1
    uint8_t portDb = ((bits & 0x01) << 4) | ((bits & 0x80) >> 6);
    uint8_t portDm = ((mask & 0x01) << 4) | ((mask & 0x80) >> 6);
    // PORT E - SRQ_PIN bit 6  in bit 6
    uint8_t portEb = (bits & 0x40);
    uint8_t portEm = (mask & 0x40);

    // Set pin states using mask
    PORTC = ( (PORTC & ~portCm) | (portCb & portCm) );
    PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
    PORTE = ( (PORTE & ~portEm) | (portEb & portEm) );

  }

}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  // most of the time, only these bits change

  if (mask & 0b00011110) {

    // PORTF - NDAC_PIN, NRFD_PIN, DAV_PIN and EOI_PIN bits 1-4 rotated into bits 4-7
    uint8_t portFb = (bits & 0x1e) << 3;
    uint8_t portFm = (mask & 0x1e) << 3;

    // Set pin direction registers using mask
    DDRF = ( (DDRF & ~portFm) | (portFb & portFm) );
  }

  if (mask & 0b11100001) {

    // PORTC - REN_PIN bit 5 rotated into bit 6
    uint8_t portCb = (bits & 0x20) << 1;
    uint8_t portCm = (mask & 0x20) << 1;
    // PORTD - IFC_PIN bit 0 rotated into bit 4 and ATN_PIN bit 7 rotated into 1
    uint8_t portDb = ((bits & 0x01) << 4) | ((bits & 0x80) >> 6);
    uint8_t portDm = ((mask & 0x01) << 4) | ((mask & 0x80) >> 6);
    // PORT E - SRQ_PIN bit 6  in bit 6
    uint8_t portEb = (bits & 0x40);
    uint8_t portEm = (mask & 0x40);

    // Set pin direction registers using mask
    DDRC = ( (DDRC & ~portCm) | (portCb & portCm) );
    DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
    DDRE = ( (DDRE & ~portEm) | (portEb & portEm) );

  }

}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif  // AVR_32U4_MICRO
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***********************************************************/



/************************************/
/***** LEONARDO R3 BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_32U4_LEO_R3
/*
  Data pin map
  ------------
  DIO1_PIN  A0 : GPIB 1  : PF7
  DIO2_PIN  A1 : GPIB 2  : PF6
  DIO3_PIN  A2 : GPIB 3  : PF5
  DIO4_PIN  A3 : GPIB 4  : PF4
  DIO5_PIN  A4 : GPIB 13 : PF3
  DIO6_PIN  A5 : GPIB 14 : PF2
  DIO7_PIN   4 : GPIB 15 : PD4
  DIO8_PIN   5 : GPIB 16 : PC6

  Control pin map
  ---------------
  IFC_PIN    8 : GPIB  9 : PB4 : b0
  NDAC_PIN   9 : GPIB  8 : PB5 : b1
  NRFD_PIN  10 : GPIB  7 : PB6 : b2
  DAV_PIN   11 : GPIB  6 : PB7 : b3
  EOI_PIN   12 : GPIB  5 : PD6 : b4
  REN_PIN    3 : GPIB 17 : PD1 : b5
  SRQ_PIN    2 : GPIB 10 : PD0 : b6
  ATN_PIN    7 : GPIB 11 : PE6 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRC |= 0b01000000;
    DDRD |= 0b00010000;
    DDRF |= 0b11110011;
    PORTC |= 0b01000000; // PORTC bits 7,5,3,1 high
    PORTD |= 0b00010000; // PORTA bits 6,4,2,0 high
    PORTF |= 0b11110011; // PORTA bits 6,4,2,0 high
    return;
  }
  
  // Set data pins to input
  DDRC &= 0b10111111 ;
  DDRD &= 0b11101111 ;
  DDRF &= 0b00001100 ;
  PORTC |= 0b01000000; // PORTD bit 4 input_pullup
  PORTD |= 0b00010000; // PORTD bit 6 input_pullup
  PORTF |= 0b11110011; // PORTC bits 7,6,5,4,1,0 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint8_t portf = reverseBits( (PINF & 0b11110000) + ((PINF & 0b00000011) << 2) );
  return ~( ((PIND & 0b00010000) << 2) + ((PINC & 0b01000000) <<1) + portf );
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
//  uint8_t rdb;
  uint8_t portf;
  // Set data pins as outputs
  DDRC |= 0b01000000;
  DDRD |= 0b00010000;
  DDRF |= 0b11110011;

  // GPIB states are inverted
  db = ~db;

  // Port F require bits mapped to 0-1 and 4-7 in reverse order
  portf = reverseBits((db & 0b00001111) + ((db & 0b00110000) << 2));

  // Set data bus
  PORTC = (PORTC & ~0b01000000) | ((db & 0b10000000) >> 1);
  PORTD = (PORTD & ~0b00010000) | ((db & 0b01000000) >> 2);
  PORTF = (PORTF & ~0b11110011) | (portf & 0b11110011);
}


/***** Set the direction and state of the GPIB control lines ****/


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  // PORTB - use bits 0 to 3, rotate bits 4 positions left to set bits 4-7 on register (pins 8-12)
  uint8_t portBb = ((bits & 0x0F) << 4);
  uint8_t portBm = ((mask & 0x0F) << 4);
  uint8_t portDb = ((bits & 0x10) << 2) + ((bits & 0x20) >> 5) + ((bits & 0x40) >> 5);
  uint8_t portDm = ((mask & 0x10) << 2) + ((mask & 0x20) >> 5) + ((mask & 0x40) >> 5);
  uint8_t portEb = ((bits & 0x80) >> 1);
  uint8_t portEm = ((mask & 0x80) >> 1);

  // Set pin states using mask
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
  PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
  PORTE = ( (PORTE & ~portEm) | (portEb & portEm) );
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  // PORTB - use bits 0 to 3, rotate bits 4 positions left to set bits 4-7 on register (pins 8-12)
  uint8_t portBb = ((bits & 0x0F) << 4);
  uint8_t portBm = ((mask & 0x0F) << 4);
  uint8_t portDb = ((bits & 0x10) << 2) + ((bits & 0x20) >> 5) + ((bits & 0x40) >> 5);
  uint8_t portDm = ((mask & 0x10) << 2) + ((mask & 0x20) >> 5) + ((mask & 0x40) >> 5);
  uint8_t portEb = ((bits & 0x80) >> 1);
  uint8_t portEm = ((mask & 0x80) >> 1);

  // Set pin direction registers using mask
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
  DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
  DDRE = ( (DDRE & ~portEm) | (portEb & portEm) );
}


uint8_t reverseBits(uint8_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif //AVR_32U4_LEO_R3
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** LEONARDO R3 BOARD LAYOUT *****/
/************************************/
