#include <Arduino.h>

#include "AR488_Config.h"
#include "AR488_Layouts.h"

/***** AR488_Hardware.cpp, ver. 0.51.28, 16/02/2024 *****/

/*
 * Hardware layout function definitions
 */

/*
volatile bool isATN_PIN = false;  // has ATN_PIN been asserted?
volatile bool isSRQ_PIN = false;  // has SRQ_PIN been asserted?
*/

/*********************************/
/***** UNO/NANO BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvv *****/
#if defined(AR488_UNO) || defined(AR488_NANO)

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
  // Set data pins to input
  DDRD &= 0b11001111 ;
  DDRC &= 0b11000000 ;
  //  PORTD = PORTD | 0b00110000; // PORTD bits 5,4 input_pullup
  //  PORTC = PORTC | 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
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


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Uno/Nano pin to Port/bit to direction/state byte map:
   IFC_PIN   8   PORTB bit 0 byte bit 0
   NDAC_PIN  9   PORTB bit 1 byte bit 1
   NRFD_PIN  10  PORTB bit 2 byte bit 2
   DAV_PIN   11  PORTB bit 3 byte bit 3
   EOI_PIN   12  PORTB bit 4 byte bit 4
   SRQ_PIN   2   PORTD bit 2 byte bit 6
   REN_PIN   3   PORTD bit 3 byte bit 5
   ATN_PIN   7   PORTD bit 8 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORTB - use only the first (right-most) 5 bits (pins 8-12)
  uint8_t portBb = bits & 0x1F;
  uint8_t portBm = mask & 0x1F;
  // PORT D - keep bit 7, rotate bit 6 right 4 positions to set bit 2 on register
  uint8_t portDb = (bits & 0x80) + ((bits & 0x40) >> 4) + ((bits & 0x20) >> 2);
  uint8_t portDm = (mask & 0x80) + ((mask & 0x40) >> 4) + ((mask & 0x20) >> 2);

  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
      PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
      DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
      break;
  }
}

#endif //AR488UNO/AR488_NANO
/***** ^^^^^^^^^^^^^^^^^^^^^ *****/
/***** UNO/NANO BOARD LAYOUT *****/
/*********************************/



/*******************************************/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_D

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
  // Set data pins to input
//  DDRD &= 0b11001111 ;
//  DDRC &= 0b11000000 ;
  DDRF &= 0b00000000 ;

//  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
//  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTF |= 0b11111111; // set PORTC bits to input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~(PINF & 0b11111111);
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
  DDRF |= 0b11111111;

  // Set data bus
  PORTF = ~db;
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout D pin to Port/bit to direction/state byte map:
   IFC_PIN   17  PORTH bit 0 byte bit 0
   NDAC_PIN  16  PORTH bit 1 byte bit 1
   NRFD_PIN  6   PORTH bit 3 byte bit 2
   DAV_PIN   7   PORTH bit 4 byte bit 3
   EOI_PIN   8   PORTH bit 5 byte bit 4
   REN_PIN   9   PORTH bit 6 byte bit 5
   // These require pcint
   SRQ_PIN   10  PORTB bit 4 byte bit 6
   ATN_PIN   11  PORTB bit 5 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORT H - keep bits 5-0. Move bits 5-2 left 1 position to set bits 6-3 and 1-0 on port
  uint8_t portHb = ((bits & 0x3C) << 1) + (bits & 0x03);
  uint8_t portHm = ((mask & 0x3C) << 1) + (mask & 0x03);

  // PORT B - keep bits 7 and 6, but rotate right 2 postions to set bits 5 and 4 on port 
  uint8_t portBb = ((bits & 0xC0) >> 2);
  uint8_t portBm = ((mask & 0xC0) >> 2);
 
  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTH = ( (PORTH & ~portHm) | (portHb & portHm) );
      PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRH = ( (DDRH & ~portHm) | (portHb & portHm) );
      DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
      break;
  }
}

#endif //MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/*******************************************/



/************************************/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_E1

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
  // Set data pins to input
  DDRA &= 0b10101010 ;
  DDRC &= 0b01010101 ;

  PORTA |= 0b01010101; // PORTA bits 6,4,2,0 input_pullup
  PORTC |= 0b10101010; // PORTC bits 7,5,3,1 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  uint8_t val = 0;

  // Read the byte of data on the bus (GPIB states are inverted)
  val = ~((PINA & 0b01010101) + (PINC & 0b10101010));

  db |= (((val >> 1) & 1)<<3);
  db |= (((val >> 3) & 1)<<2);
  db |= (((val >> 5) & 1)<<1);
  db |= (((val >> 7) & 1)<<0);

  db |= (((val >> 6) & 1)<<7);
  db |= (((val >> 4) & 1)<<6);
  db |= (((val >> 2) & 1)<<5);
  db |= (((val >> 0) & 1)<<4);

  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint8_t val = 0;
  
  // Set data pins as outputs
  DDRA |= 0b01010101 ;
  DDRC |= 0b10101010 ;

  // GPIB states are inverted
  db = ~db;

  val |= (((db >> 3) & 1)<<1);
  val |= (((db >> 2) & 1)<<3);
  val |= (((db >> 1) & 1)<<5);
  val |= (((db >> 0) & 1)<<7);

  val |= (((db >> 7) & 1)<<6);
  val |= (((db >> 6) & 1)<<4);
  val |= (((db >> 5) & 1)<<2);
  val |= (((db >> 4) & 1)<<0);

  // Set data bus
  PORTA = (PORTA & ~0b01010101) | (val & 0b01010101);
  PORTC = (PORTC & ~0b10101010) | (val & 0b10101010);
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout E1 pin to Port/bit to direction/state byte map:
   IFC_PIN   48  PORTL bit 1 byte bit 0
   NDAC_PIN  46  PORTL bit 3 byte bit 1
   NRFD_PIN  44  PORTL bit 5 byte bit 2
   DAV_PIN   42  PORTL bit 7 byte bit 3
   EOI_PIN   40  PORTG bit 1 byte bit 4
   REN_PIN   38  PORTD bit 7 byte bit 5
   // These require pcint
   SRQ_PIN   50  PORTB bit 3 byte bit 6
   ATN_PIN   52  PORTB bit 1 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORT B
  uint8_t portBb = (((bits >> 7 & 1))<<1) + (((bits >> 6 & 1))<<3);
  uint8_t portBm = (((mask >> 7 & 1))<<1) + (((mask >> 6 & 1))<<3);

  // PORT D
  uint8_t portDb = (((bits >> 5 & 1))<<7);
  uint8_t portDm = (((mask >> 5 & 1))<<7);

  // PORT G
  uint8_t portGb = (((bits >> 4 & 1))<<1);
  uint8_t portGm = (((mask >> 4 & 1))<<1);

  // PORT L
  uint8_t portLb = (((bits >> 0 & 1))<<1) + (((bits >> 1 & 1))<<3) + (((bits >> 2 & 1))<<5) + (((bits >> 3 & 1))<<7);
  uint8_t portLm = (((mask >> 0 & 1))<<1) + (((mask >> 1 & 1))<<3) + (((mask >> 2 & 1))<<5) + (((mask >> 3 & 1))<<7);

  // Set PORTs using mask to avoid affecting bits that should not be affected
  // and calculated and masked port byte
  // PORT B - bits 7 & 6 (ATN_PIN + SRQ_PIN)
  // PORT D - bit 5 (REN_PIN)
  // PORT G - bit 4 (EOI_PIN)
  // PORT L - bits 1,3,5,7 (IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN)
  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
      PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
      PORTG = ( (PORTG & ~portGm) | (portGb & portGm) );
      PORTL = ( (PORTL & ~portLm) | (portLb & portLm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
      DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
      DDRG = ( (DDRG & ~portGm) | (portGb & portGm) );
      DDRL = ( (DDRL & ~portLm) | (portLb & portLm) );
      break;
  }
}

#endif //MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/************************************/



/************************************/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_E2

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {

  // Set data pins to input
  DDRA &= 0b01010101 ;
  DDRC &= 0b10101010 ;

  PORTA |= 0b10101010; // PORTC bits 7,5,3,1 input_pullup
  PORTC |= 0b01010101; // PORTA bits 6,4,2,0 input_pullup
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  uint8_t val = 0;
  
  // Read the byte of data on the bus (GPIB states are inverted)
  val = ~((PINA & 0b10101010) + (PINC & 0b01010101));

  db |= (((val >> 0) & 1)<<3);
  db |= (((val >> 2) & 1)<<2);
  db |= (((val >> 4) & 1)<<1);
  db |= (((val >> 6) & 1)<<0);

  db |= (((val >> 7) & 1)<<7);
  db |= (((val >> 5) & 1)<<6);
  db |= (((val >> 3) & 1)<<5);
  db |= (((val >> 1) & 1)<<4);

  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint8_t val = 0;
  
  // Set data pins as outputs
  DDRA |= 0b10101010 ;
  DDRC |= 0b01010101 ;

  // GPIB states are inverted
  db = ~db;

  val |= (((db >> 4) & 1)<<1);
  val |= (((db >> 5) & 1)<<3);
  val |= (((db >> 6) & 1)<<5);
  val |= (((db >> 7) & 1)<<7);

  val |= (((db >> 0) & 1)<<6);
  val |= (((db >> 1) & 1)<<4);
  val |= (((db >> 2) & 1)<<2);
  val |= (((db >> 3) & 1)<<0);

  // Set data bus
  PORTA = (PORTA & ~0b10101010) | (val & 0b10101010);
  PORTC = (PORTC & ~0b01010101) | (val & 0b01010101);
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout E2 pin to Port/bit to direction/state byte map:
   IFC_PIN   48  PORTL bit 1 byte bit 0
   NDAC_PIN  46  PORTL bit 3 byte bit 1
   NRFD_PIN  44  PORTL bit 5 byte bit 2
   DAV_PIN   42  PORTL bit 7 byte bit 3
   EOI_PIN   40  PORTG bit 1 byte bit 4
   REN_PIN   38  PORTD bit 7 byte bit 5
   // These require pcint
   SRQ_PIN   50  PORTB bit 3 byte bit 6
   ATN_PIN   52  PORTB bit 1 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORT B
  uint8_t portBb = (((bits >> 7 & 1))<<0) + (((bits >> 6 & 1))<<2);
  uint8_t portBm = (((mask >> 7 & 1))<<0) + (((mask >> 6 & 1))<<2);

  // PORT G
  uint8_t portGb = (((bits >> 4 & 1))<<0) + (((bits >> 5 & 1))<<2);
  uint8_t portGm = (((mask >> 4 & 1))<<0) + (((mask >> 5 & 1))<<2);

  // PORT L
  uint8_t portLb = (((bits >> 0 & 1))<<0) + (((bits >> 1 & 1))<<2) + (((bits >> 2 & 1))<<4) + (((bits >> 3 & 1))<<6);
  uint8_t portLm = (((mask >> 0 & 1))<<0) + (((mask >> 1 & 1))<<2) + (((mask >> 2 & 1))<<4) + (((mask >> 3 & 1))<<6);

  // Set PORTs using mask to avoid affecting bits that should not be affected
  // and calculated and masked port byte
  // PORT B - bits 0 & 2 (ATN_PIN + SRQ_PIN)
  // PORT G - bits 0 & 2 (EOI_PIN, REN_PIN)
  // PORT L - bits 0,2,4,6 (IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN)
  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
      PORTG = ( (PORTG & ~portGm) | (portGb & portGm) );
      PORTL = ( (PORTL & ~portLm) | (portLb & portLm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
      DDRG = ( (DDRG & ~portGm) | (portGb & portGm) );
      DDRL = ( (DDRL & ~portLm) | (portLb & portLm) );
      break;
  }
}

#endif //MEGA2560

/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/************************************/



/***********************************************************/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4_MICRO

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
  // Set data pins to input
  DDRB  &= 0b10000001 ;
  DDRD  &= 0b01111110 ;
  PORTB |= 0b01111110; // PORTB bits 6,5,4,3,2,1 input_pullup
  PORTD |= 0b10000001; // PORTD bits 7,0 input_pullup

  // Read the byte of data on the bus
  // DIO8_PIN -> PORTD bit 7, DIO7_PIN -> PORTE bit 5, DIO6_PIN-DIO1_PIN -> PORTB bit 451326
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  return ~((PIND & 0b10000001) | (PINB & 0b01111110)) ;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  //Serial.print("dbus 0x");
  //Serial.println(db, HEX);

  // Set data pins as outputs
  DDRB |= 0b01111110;
  DDRD |= 0b10000001;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTB = (PORTB & ~0b01111110) | (db & 0b01111110) ;
  PORTD = (PORTD & ~0b10000001) | (db & 0b10000001);
}


/***** Set the direction and state of the GPIB control lines *****/
/*
 * Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
 * Arduino Pro Micro pin to Port/bit to direction/state byte map:
 * IFC_PIN   4   PORTD bit 4   byte bit 0
 * NDAC_PIN  A3  PORTF bit 4   byte bit 1
 * NRFD_PIN  A2  PORTF bit 5   byte bit 2
 * DAV_PIN   A1  PORTF bit 6   byte bit 3
 * EOI_PIN   A0  PORTF bit 7   byte bit 4
 * REN_PIN   5   PORTC bit 6   byte bit 5
 * SRQ_PIN   7   PORTE bit 6   byte bit 6
 * ATN_PIN   2   PORTD bit 1   byte bit 7
 * 
 * It would be more efficient (and easier to read the code) if the bits in the above
 * control word were assigned by name to match suitable port bits : then NDAC_PIN,NRFD_PIN and DAV_PIN
 * could be positioned at bits 4,5,6 to be placed in port F without shifting.
 */
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // most of the time, only these bits change

  if (mask & 0b00011110) {

    // PORTF - NDAC_PIN, NRFD_PIN, DAV_PIN and EOI_PIN bits 1-4 rotated into bits 4-7
    uint8_t portFb = (bits & 0x1e) << 3;
    uint8_t portFm = (mask & 0x1e) << 3;

    // Set registers: register = (register & ~bitmask) | (value & bitmask)
    // Mask: 0=unaffected; 1=to be changed

    switch (mode) {
      case 0:
        // Set pin states using mask
        PORTF = ( (PORTF & ~portFm) | (portFb & portFm) );
        break;
      case 1:
        // Set pin direction registers using mask
        DDRF = ( (DDRF & ~portFm) | (portFb & portFm) );
        break;
    }
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

    // Set registers: register = (register & ~bitmask) | (value & bitmask)
    // Mask: 0=unaffected; 1=to be changed

    switch (mode) {
      case 0:
        // Set pin states using mask
        PORTC = ( (PORTC & ~portCm) | (portCb & portCm) );
        PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
        PORTE = ( (PORTE & ~portEm) | (portEb & portEm) );
        break;
      case 1:
        // Set pin direction registers using mask
        DDRC = ( (DDRC & ~portCm) | (portCb & portCm) );
        DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
        DDRE = ( (DDRE & ~portEm) | (portEb & portEm) );
        break;
    }
  }
}

#endif  // AR488_MEGA32U4_MICRO
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***********************************************************/



/************************************/
/***** LEONARDO R3 BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4_LR3

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
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
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Leonardo R3 pin to Port/bit to direction/state byte map:
   IFC_PIN   8   PORTB bit 4 byte bit 0
   NDAC_PIN  9   PORTB bit 5 byte bit 1
   NRFD_PIN  10  PORTB bit 6 byte bit 2
   DAV_PIN   11  PORTB bit 7 byte bit 3
   EOI_PIN   12  PORTD bit 6 byte bit 4
   SRQ_PIN   2   PORTD bit 1 byte bit 6
   REN_PIN   3   PORTD bit 0 byte bit 5
   ATN_PIN   7   PORTE bit 6 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORTB - use bits 0 to 3, rotate bits 4 positions left to set bits 4-7 on register (pins 8-12)
  uint8_t portBb = ((bits & 0x0F) << 4);
  uint8_t portBm = ((mask & 0x0F) << 4);
  // PORTD - use bit 4, rotate left 2 positions to set bit 6 on register (EOI_PIN)
  // PORTD - use bit 5, rotate right 5 positions to set bit 0 on register (REN_PIN)
  // PORTD - use bit 6, rotate right 5 positions to set bit 1 on register (SRQ_PIN)
  uint8_t portDb = ((bits & 0x10) << 2) + ((bits & 0x20) >> 5) + ((bits & 0x40) >> 5);
  uint8_t portDm = ((mask & 0x10) << 2) + ((mask & 0x20) >> 5) + ((mask & 0x40) >> 5);
  // PORTE - use bit 7, rotate left 1 position to set bit 6 on register (ATN_PIN)
  uint8_t portEb = ((bits & 0x80) >> 1);
  uint8_t portEm = ((mask & 0x80) >> 1);

  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
      PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
      PORTE = ( (PORTE & ~portEm) | (portEb & portEm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
      DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
      DDRE = ( (DDRE & ~portEm) | (portEb & portEm) );
      break;
  }
}


uint8_t reverseBits(uint8_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}

#endif //AR488_MEGA32U4_LR3
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** LEONARDO R3 BOARD LAYOUT *****/
/************************************/



/******************************************/
/***** MCP23S17 EXPANDER (SPI) LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MCP23S17


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
void readyGpibDbus() {
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


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   MCP23S17 pin to Port/bit to direction/state byte map:
   IFC_PIN   0   PORTA bit 0 byte bit 0
   NDAC_PIN  1   PORTA bit 1 byte bit 1
   NRFD_PIN  2   PORTA bit 2 byte bit 2
   DAV_PIN   3   PORTA bit 3 byte bit 3
   EOI_PIN   4   PORTA bit 4 byte bit 4
   REN_PIN   5   PORTA bit 5 byte bit 5
   SRQ_PIN   6   PORTA bit 6 byte bit 6
   ATN_PIN   7   PORTA bit 7 byte bit 7
*/

void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  uint8_t portAb = bits;
  uint8_t portAm = mask;

  uint8_t regByte = 0;
  uint8_t regMod = 0; 


  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:

      // Set pin states using mask
      regByte = mcpByteRead(MCPPORTA);
      regMod = (regByte & ~portAm) | (portAb & portAm);
      mcpByteWrite(MCPPORTA, regMod);
      break;

    case 1:
      // Set pin direction registers using mask
      regByte = ~mcpByteRead(MCPDIRA);   // Note: on MCP23S17 0 = output, 1 = input
      regMod = (regByte & ~portAm) | (portAb & portAm);
      mcpByteWrite(MCPDIRA, ~regMod);    // Note: on MCP23S17 0 = output, 1 = input
      break;

  }
}


/***** MCP23S17 interrupt handler *****/
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


/***** Read from the MCP23S17 *****/
/*
 * reg : register we want to read , e.g. MCPPORTA or MCPPORTB
 */
uint8_t mcpByteRead(uint8_t reg){
  uint8_t db;
//Serial.print(F("Rx addr: "));
//Serial.print(mcpAddr);
  digitalWrite(chipSelect, LOW);            // Enable MCP communication
  SPI.transfer(MCPREAD | (mcpAddr << 1));   // Write opcode + chip address + write bit
  SPI.transfer(reg);                        // Write the register we want to read
  db = SPI.transfer(0x00);                  // Send any byte. Function returns low byte (port A value) which is ignored
  digitalWrite(chipSelect, HIGH);           // Enable MCP communication
//uint8_t dbinv = ~db;
//Serial.print(F("\tRCV: "));
//Serial.println(dbinv, HEX);
  return db;
}


/***** Write to the MCP23S17 *****/
void mcpByteWrite(uint8_t reg, uint8_t db){
//Serial.print(F("Tx addr: "));
//Serial.print(mcpAddr);
  digitalWrite(chipSelect, LOW);            // Enable MCP communication
  SPI.transfer(MCPWRITE | (mcpAddr << 1));  // Write opcode (with write bit set) + chip address
  SPI.transfer(reg);                        // Write register we want to change
  SPI.transfer(db);                         // Write data byte
  digitalWrite(chipSelect, HIGH);           // Stop MCP communication
//Serial.print(F("\tWRT: "));
//Serial.print(db, HEX);
//uint8_t dbinv = ~db;
//Serial.print(F("\tWRTinv: "));
//Serial.println(dbinv, HEX);
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

#endif //AR488_MCP23S17
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MCP23S17 EXPANDER (SPI) LAYOUT *****/
/******************************************/



/***************************************************/
/***** PANDUINO/MIGHTYCORE MCGRAW BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA644P_MCGRAW

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
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


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Uno/Nano pin to Port/bit to direction/state byte map:
   IFC_PIN   22  PORTC bit 6 byte bit 0
   NDAC_PIN  21  PORTC bit 5 byte bit 1
   NRFD_PIN  20  PORTC bit 4 byte bit 2
   DAV_PIN   19  PORTC bit 3 byte bit 3
   EOI_PIN   18  PORTC bit 2 byte bit 4
   SRQ_PIN   23  PORTC bit 7 byte bit 6
   REN_PIN   24  PORTA bit 0 byte bit 5
   ATN_PIN   31  PORTA bit 7 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORT A - use bits 5 and 7. Map to port A bits 0 and 7
  uint8_t portAb = ((bits & 0x20) >> 5) + (bits &  0x80);
  uint8_t portAm = ((mask & 0x20) >> 5) + (mask &  0x80);

  // PORT C- use the 5 right-most bits (bits 0 - 4) and bit 6
  // Reverse bits 0-4 and map to bits 2-6. Map bit 6 to bit 7
  uint8_t portCb = (reverseBits(bits & 0x1F) >> 1) + ((bits & 0x40) << 1);
  uint8_t portCm = (reverseBits(mask & 0x1F) >> 1) + ((mask & 0x40) << 1);

  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  switch (mode) {
    case 0:
      // Set pin states using mask
      PORTA = ( (PORTA & ~portAm) | (portAb & portAm) );
      PORTC = ( (PORTC & ~portCm) | (portCb & portCm) );
      break;
    case 1:
      // Set pin direction registers using mask
      DDRA = ( (DDRA & ~portAm) | (portAb & portAm) );
      DDRC = ( (DDRC & ~portCm) | (portCb & portCm) );
      break;
  }
}


#endif // AR488_MEGA644P_MCGRAW
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PANDUINO/MIGHTYCORE MCGRAW BOARD LAYOUT *****/
/***************************************************/



/*************************************/
/***** CUSTOM PIN LAYOUT SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#if defined (AR488_CUSTOM) || defined (NON_ARDUINO)

uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus() {
  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], INPUT_PULLUP);
  }
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    db = db + (digitalRead(databus[i]) ? 0 : 1<<i );
  }
  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], OUTPUT);
    digitalWrite(databus[i], ((db&(1<<i)) ? LOW : HIGH) );
  }
  
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
   state: 0=LOW; 1=HIGH/INPUT_PULLUP
   dir  : 0=input; 1=output;
   mode:  0=set pin state; 1=set pin direction
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  switch (mode) {
    case 0:
      // Set pin state
      for (uint8_t i=0; i<8; i++) {
        if (mask&(1<<i)) digitalWrite( ctrlbus[i], ((bits&(1<<i)) ? HIGH : LOW) );
      }
      break;
    case 1:
      // Set pin direction
      for (uint8_t i=0; i<8; i++) {
        if (mask&(1<<i)) pinMode( ctrlbus[i], ((bits&(1<<i)) ? OUTPUT : INPUT_PULLUP) );
      }
      break;
  }

}

#endif
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** CUSTOM PIN LAYOUT SECTION *****/
/*************************************/




/************************************/
/***** COMMON FUNCTIONS SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/

#if not defined(AR488_MCP23S17) && not defined(AR488_MCP23017)
uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}
#endif

/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** COMMON FUNCTIONS SECTION *****/
/************************************/
