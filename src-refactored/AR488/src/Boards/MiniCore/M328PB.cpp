#include "M328PB.h"



/*****************************************/
/***** UNO/NANO LAYOUT FOR THE 328PB *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef MINICORE_NANO


/***** Control pin map *****/
/*
  Data pin map
  ------------
  DIO1_PIN  A0 : GPIB 1  : PC0
  DIO2_PIN  A1 : GPIB 2  : PC1
  DIO3_PIN  A2 : GPIB 3  : PC2
  DIO4_PIN  A3 : GPIB 4  : PC3
  DIO5_PIN  A4 : GPIB 13 : PC4
  DIO6_PIN  A5 : GPIB 14 : PC5
  DIO7_PIN   4 : GPIB 15 : PD4
  DIO8_PIN   5 : GPIB 16 : PD5

  Control pin map
  ---------------
  IFC_PIN   8  : GPIB  9 : PB0 : b0
  NDAC_PIN  9  : GPIB  8 : PB1 : b1
  NRFD_PIN  10 : GPIB  7 : PB2 : b2
  DAV_PIN   11 : GPIB  6 : PB3 : b3
  EOI_PIN   12 : GPIB  5 : PB4 : b4
  REN_PIN   3  : GPIB 17 : PD2 : b5
  SRQ_PIN   2  : GPIB 10 : PD2 : b6
  ATN_PIN   7  : GPIB 11 : PD7 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=LOW, 1=HIGH
    mask : 0=unaffected, 1=affected
*/



/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRD |= 0b00110000;
    DDRC |= 0b00111111;
    PORTD |= 0b00110000; // PORTC bits 7,5,3,1 high
    PORTC |= 0b00111111; // PORTA bits 6,4,2,0 high
    return;
  }

  // Set data pins to input
  DDRD &= 0b11001111 ;
  DDRC &= 0b11000000 ;
  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~((PIND << 2 & 0b11000000) + (PINC & 0b00111111));
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
  DDRD |= 0b00110000;
  DDRC |= 0b00111111;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTC = (PORTC & ~0b00111111) | (db & 0b00111111);
  PORTD = (PORTD & ~0b00110000) | ((db & 0b11000000) >> 2);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  if (!mask) return;  // Empty mask does nothing!
  // PORTB - use only the first (right-most) 5 bits (pins 8-12)
  uint8_t portBb = bits & 0x1F;
  uint8_t portBm = mask & 0x1F;
  // PORT D - keep bit 7, rotate bit 6 right 4 positions to set bit 2 on register
  uint8_t portDb = (bits & 0x80) + ((bits & 0x40) >> 4) + ((bits & 0x20) >> 2);
  uint8_t portDm = (mask & 0x80) + ((mask & 0x40) >> 4) + ((mask & 0x20) >> 2);

  // Set pin states using mask
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
  PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  // PORTB - use only the first (right-most) 5 bits (pins 8-12)
  uint8_t portBb = bits & 0x1F;
  uint8_t portBm = mask & 0x1F;
  // PORT D - keep bit 7, rotate bit 6 right 4 positions to set bit 2 on register
  uint8_t portDb = (bits & 0x80) + ((bits & 0x40) >> 4) + ((bits & 0x20) >> 2);
  uint8_t portDm = (mask & 0x80) + ((mask & 0x40) >> 4) + ((mask & 0x20) >> 2);

  uint8_t pmask;

  // Set pin direction registers using mask
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
  DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );

  // Set inputs to input_pullup
  pmask = (~portBb & portBm);
  PORTB = ( (PORTB & ~pmask) | pmask );
  pmask = (~portDb & portDm);
  PORTD = ( (PORTD & ~pmask) | pmask );

}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif  // MINICORE_NANO
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** UNO/NANO LAYOUT FOR THE 328PB *****/
/******************************************/



/******************************************/
/***** POLOLU A-STAR 328PB ALT LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef MINICORE_NANO_ALT

/*
  Data pin map
  ------------
  DIO1_PIN  A0 : GPIB 1  : PC0
  DIO2_PIN  A1 : GPIB 2  : PC1
  DIO3_PIN  A2 : GPIB 3  : PC2
  DIO4_PIN  A3 : GPIB 4  : PC3
  DIO5_PIN  A4 : GPIB 13 : PC4
  DIO6_PIN  A5 : GPIB 14 : PC5
  DIO7_PIN  A6 : GPIB 15 : PD4
  DIO8_PIN  A7 : GPIB 16 : PD5

  Control pin map
  ---------------
  IFC_PIN    5 : GPIB  9 : PD5 : b0
  NDAC_PIN   6 : GPIB  8 : PD6 : b1
  NRFD_PIN   7 : GPIB  7 : PD7 : b2
  DAV_PIN    8 : GPIB  6 : PB0 : b3
  EOI_PIN    9 : GPIB  5 : PB1 : b4
  REN_PIN    2 : GPIB 17 : PD2 : b5
  SRQ_PIN    3 : GPIB 10 : PD3 : b6
  ATN_PIN    4 : GPIB 11 : PD4 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRC |= 0b00111111;
    DDRE |= 0b00001100;
    PORTC |= 0b00111111; // PORTC bits 7,5,3,1 high
    PORTE |= 0b00001100; // PORTA bits 6,4,2,0 high
    return;
  }
  // Set data pins to input
  DDRC &= 0b11000000;
  DDRE &= 0b11110011;
  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTE |= 0b00001100; // PORTE bits 3,2 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~((PINE << 4 & 0b11000000) + (PINC & 0b00111111));
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
  DDRE |= 0b00001100;
  DDRC |= 0b00111111;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTC = (PORTC & ~0b00111111) | (db & 0b00111111);
  PORTE = (PORTE & ~0b00001100) | ((db & 0b11000000) >> 4);
}


/***** Set the state of the GPIB control lines ****/
void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  if (!mask) return;  // Empty mask does nothing!

  // PORTB - take bits 3 and 4 or value and move to the first two bits (pins 8, 9)
  uint8_t portBb = (bits & 0x18) >> 3;
  uint8_t portBm = (mask & 0x18) >> 3;
  // PORT D - take bits 0-2, move to 5-7, take bits 5-7 and move to 2-4
  uint8_t portDb = ((bits & 0x07) << 5) + ((bits & 0xE0) >> 3);
  uint8_t portDm = ((mask & 0x07) << 5) + ((mask & 0xE0) >> 3);

  // Set pin states using mask
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
  PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
}


/***** Set the direction of the GPIB control lines ****/
void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  uint8_t pmask;
  
  // PORTB - take bits 3 and 4 or value and move to the first two bits (pins 8, 9)
  uint8_t portBb = (bits & 0x18) >> 3;
  uint8_t portBm = (mask & 0x18) >> 3;
  // PORT D - take bits 0-2, move to 5-7, take bits 5-7 and move to 2-4
  uint8_t portDb = ((bits & 0x07) << 5) + ((bits & 0xE0) >> 3);
  uint8_t portDm = ((mask & 0x07) << 5) + ((mask & 0xE0) >> 3);

  // Set pin direction registers using mask
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
  DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );

  // Set inputs to input_pullup
  pmask = (~portBb & portBm);
  PORTB = ( (PORTB & ~pmask) | pmask );
  pmask = (~portDb & portDm);
  PORTD = ( (PORTD & ~pmask) | pmask );

}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}


#endif  // MINICORE_NANO_ALT
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** POLOLU A-STAR 328PB ALT LAYOUT *****/
/******************************************/

