#include <Arduino.h>

#include "AR488_Config.h"
#include "AR488_Layouts.h"

/***** AR488_Hardware.cpp, ver. 0.49.08, 22/12/2020 *****/
/*
 * Hardware layout function definitions
 */

volatile bool isATN = false;  // has ATN been asserted?
volatile bool isSRQ = false;  // has SRQ been asserted?


/*********************************/
/***** UNO/NANO BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvv *****/
#if defined(AR488_UNO) || defined(AR488_NANO)

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
void readyGpibDbus() {
  // Set data pins to input
  DDRD &= 0b11001111 ;
  DDRC &= 0b11000000 ;
  //  PORTD = PORTD | 0b00110000; // PORTD bits 5,4 input_pullup
  //  PORTC = PORTC | 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
}

uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~((PIND << 2 & 0b11000000) + (PINC & 0b00111111));
}


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
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
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Uno/Nano pin to Port/bit to direction/state byte map:
   IFC   8   PORTB bit 0 byte bit 0
   NDAC  9   PORTB bit 1 byte bit 1
   NRFD  10  PORTB bit 2 byte bit 2
   DAV   11  PORTB bit 3 byte bit 3
   EOI   12  PORTB bit 4 byte bit 4
   SRQ   2   PORTD bit 2 byte bit 6
   REN   3   PORTD bit 3 byte bit 5
   ATN   7   PORTD bit 8 byte bit 7
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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

volatile uint8_t atnPinMem = ATNPREG;
volatile uint8_t srqPinMem = SRQPREG;
static const uint8_t ATNint = 0b10000000;
static const uint8_t SRQint = 0b00000100;


void interruptsEn(){
  cli();
  PCICR |= 0b00000100;  // PORTD
  PCMSK2 |= (SRQint^ATNint);
  sei();
}

#pragma GCC diagnostic error "-Wmisspelled-isr"


/***** Interrupt handler *****/
ISR(PCINT2_vect) {

  // Has PCINT23 fired (ATN asserted)?
  if ((PIND ^ atnPinMem) & ATNint) {
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has PCINT19 fired (SRQ asserted)?
  if ((PIND ^ srqPinMem) & SRQint) {
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of PORTD register
  atnPinMem = ATNPREG;
  srqPinMem = SRQPREG;
}


/***** Catchall interrupt vector *****/
/*
  ISR(BADISR_vect) {
  // ISR to catch ISR firing without handler
  isBAD = true;
  }
*/
#endif //USE_INTERRUPTS


#endif //AR488UNO/AR488_NANO
/***** ^^^^^^^^^^^^^^^^^^^^^ *****/
/***** UNO/NANO BOARD LAYOUT *****/
/*********************************/



/*******************************************/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_D

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
void readyGpibDbus() {
  // Set data pins to input
//  DDRD &= 0b11001111 ;
//  DDRC &= 0b11000000 ;
  DDRF &= 0b00000000 ;

//  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
//  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTF |= 0b11111111; // set PORTC bits to input_pullup
}

uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  return ~(PINF & 0b11111111);
}


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
void setGpibDbus(uint8_t db) {
  // Set data pins as outputs
//  DDRD |= 0b00110000;
//  DDRC |= 0b00111111;

  DDRF |= 0b11111111;

  // GPIB states are inverted
//  db = ~db;

  // Set data bus
//  PORTC = (PORTC & ~0b00111111) | (db & 0b00111111);
//  PORTD = (PORTD & ~0b00110000) | ((db & 0b11000000) >> 2);

  PORTF = ~db;
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout D pin to Port/bit to direction/state byte map:
   IFC   17  PORTH bit 0 byte bit 0
   NDAC  16  PORTH bit 1 byte bit 1
   NRFD  6   PORTH bit 3 byte bit 2
   DAV   7   PORTH bit 4 byte bit 3
   EOI   8   PORTH bit 5 byte bit 4
   REN   9   PORTH bit 6 byte bit 5
   // These require pcint
   SRQ   10  PORTB bit 4 byte bit 6
   ATN   11  PORTB bit 5 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORT H - keep bits 5-0. Move bits 5-2 left 1 position to set bits 6-3 and 1-0 on port
  uint8_t portHb = ((bits & 0x3C) << 1) + (bits & 0x03);
  uint8_t portHm = ((mask & 0x3C) << 1) + (mask & 0x03);
  // PORT D - keep bit 7, rotate bit 6 right 4 positions to set bit 2 on register
//  uint8_t portDb = (bits & 0x80) + ((bits & 0x40) >> 4) + ((bits & 0x20) >> 2);
//  uint8_t portDm = (mask & 0x80) + ((mask & 0x40) >> 4) + ((mask & 0x20) >> 2);

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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

volatile uint8_t atnPinMem = ATNPREG;
volatile uint8_t srqPinMem = SRQPREG;
static const uint8_t ATNint = 0b00100000;
static const uint8_t SRQint = 0b00010000;


void interruptsEn(){
  cli();
  PCICR |= 0b00000001;  // PORTB
  PCMSK0 |= (SRQint^ATNint);
  sei();
}


#pragma GCC diagnostic error "-Wmisspelled-isr"

/***** Interrupt handler *****/
ISR(PCINT0_vect) {

  // Has PCINT5 fired (ATN asserted)?
  if ((ATNPREG ^ atnPinMem) & ATNint) {
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has PCINT4 fired (SRQ asserted)?
  if ((SRQPREG ^ srqPinMem) & SRQint) {
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of PORTD register
  atnPinMem = ATNPREG;
  srqPinMem = SRQPREG;
}

#endif //USE_INTERRUPTS

#endif //MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/*******************************************/


/************************************/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_E1

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
void readyGpibDbus() {
  // Set data pins to input
  DDRA &= 0b10101010 ;
  DDRC &= 0b01010101 ;

  PORTA |= 0b01010101; // PORTA bits 6,4,2,0 input_pullup
  PORTC |= 0b10101010; // PORTC bits 7,5,3,1 input_pullup
}

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


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
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
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout E1 pin to Port/bit to direction/state byte map:
   IFC   48  PORTL bit 1 byte bit 0
   NDAC  46  PORTL bit 3 byte bit 1
   NRFD  44  PORTL bit 5 byte bit 2
   DAV   42  PORTL bit 7 byte bit 3
   EOI   40  PORTG bit 1 byte bit 4
   REN   38  PORTD bit 7 byte bit 5
   // These require pcint
   SRQ   50  PORTB bit 3 byte bit 6
   ATN   52  PORTB bit 1 byte bit 7
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
  // PORT B - bits 7 & 6 (ATN + SRQ)
  // PORT D - bit 5 (REN)
  // PORT G - bit 4 (EOI)
  // PORT L - bits 1,3,5,7 (IFC, NDAC, NRFD, DAV)
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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

volatile uint8_t atnPinMem = ATNPREG;
volatile uint8_t srqPinMem = SRQPREG;
static const uint8_t ATNint = 0b00000010;
static const uint8_t SRQint = 0b00001000;


void interruptsEn(){
  cli();
  PCICR |= 0b00000001;  // PORTB
  PCMSK0 |= (SRQint^ATNint);
  sei();
}


#pragma GCC diagnostic error "-Wmisspelled-isr"

/***** Interrupt handler *****/
ISR(PCINT0_vect) {

  // Has PCINT1 fired (ATN asserted)?
  if ((ATNPREG ^ atnPinMem) & ATNint) {
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has PCINT3 fired (SRQ asserted)?
  if ((SRQPREG ^ srqPinMem) & SRQint) {
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of PORTD register
  atnPinMem = ATNPREG;
  srqPinMem = SRQPREG;
}

#endif //USE_INTERRUPTS

#endif //MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/************************************/


/************************************/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560_E2

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
void readyGpibDbus() {

  // Set data pins to input
  DDRA &= 0b01010101 ;
  DDRC &= 0b10101010 ;

  PORTA |= 0b10101010; // PORTC bits 7,5,3,1 input_pullup
  PORTC |= 0b01010101; // PORTA bits 6,4,2,0 input_pullup
}

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


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
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
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Mega 2560 Layout E2 pin to Port/bit to direction/state byte map:
   IFC   48  PORTL bit 1 byte bit 0
   NDAC  46  PORTL bit 3 byte bit 1
   NRFD  44  PORTL bit 5 byte bit 2
   DAV   42  PORTL bit 7 byte bit 3
   EOI   40  PORTG bit 1 byte bit 4
   REN   38  PORTD bit 7 byte bit 5
   // These require pcint
   SRQ   50  PORTB bit 3 byte bit 6
   ATN   52  PORTB bit 1 byte bit 7
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
  // PORT B - bits 0 & 2 (ATN + SRQ)
  // PORT G - bits 0 & 2 (EOI, REN)
  // PORT L - bits 0,2,4,6 (IFC, NDAC, NRFD, DAV)
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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

volatile uint8_t atnPinMem = ATNPREG;
volatile uint8_t srqPinMem = SRQPREG;
static const uint8_t ATNint = 0b00000001;
static const uint8_t SRQint = 0b00000100;


void interruptsEn(){
  cli();
  PCICR |= 0b00000001;  // PORTB
  PCMSK0 |= (SRQint^ATNint);
  sei();
}


#pragma GCC diagnostic error "-Wmisspelled-isr"

/***** Interrupt handler *****/
ISR(PCINT0_vect) {

  // Has PCINT0 fired (ATN asserted)?
  if ((ATNPREG ^ atnPinMem) & ATNint) {
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has PCINT2 fired (SRQ asserted)?
  if ((SRQPREG ^ srqPinMem) & SRQint) {
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of PORTD register
  atnPinMem = ATNPREG;
  srqPinMem = SRQPREG;
}

#endif //USE_INTERRUPTS

#endif //MEGA2560

/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/************************************/



/***********************************************************/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4_MICRO

void readyGpibDbus() {
  // Set data pins to input
  DDRB  &= 0b10000001 ;
  DDRD  &= 0b01111110 ;
  PORTB |= 0b01111110; // PORTB bits 6,5,4,3,2,1 input_pullup
  PORTD |= 0b10000001; // PORTD bits 7,0 input_pullup

  // Read the byte of data on the bus
  // DIO8 -> PORTD bit 7, DIO7 -> PORTE bit 5, DIO6-DIO1 -> PORTB bit 451326

#ifdef MICRODEBUG
  Serial.print("B ");
  Serial.print(PINB & 0x7e, HEX);
  Serial.print(", D ");
  Serial.print(PIND & 0x81, HEX);

  uint8_t x = ~((PIND & 0b10000001) | (PINB & 0b01111110)) ;
  Serial.print(" value ");
  Serial.println(x);
#endif
}

uint8_t readGpibDbus() {
  return ~((PIND & 0b10000001) | (PINB & 0b01111110)) ;
}


/***** Set the status of the GPIB data bus wires with a byte of data *****/
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

#ifdef MICRODEBUG
  Serial.print("bits B ");
  Serial.print(db & 0b01111110, HEX);
  Serial.print(", bits D ");
  Serial.println(db & 0b10000001, HEX);
#endif
}

/***** Set the direction and state of the GPIB control lines *****/
/*
 * Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
 * Arduino Pro Micro pin to Port/bit to direction/state byte map:
 * IFC   4   PORTD bit 4   byte bit 0
 * NDAC  A3  PORTF bit 4   byte bit 1
 * NRFD  A2  PORTF bit 5   byte bit 2
 * DAV   A1  PORTF bit 6   byte bit 3
 * EOI   A0  PORTF bit 7   byte bit 4
 * REN   5   PORTC bit 6   byte bit 5
 * SRQ   7   PORTE bit 6   byte bit 6
 * ATN   2   PORTD bit 1   byte bit 7
 * 
 * It would be more efficient (and easier to read the code) if the bits in the above
 * control word were assigned by name to match suitable port bits : then NDAC,NRFD and DAV
 * could be positioned at bits 4,5,6 to be placed in port F without shifting.
 */
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // most of the time, only these bits change

  if (mask & 0b00011110) {

    // PORTF - NDAC, NRFD, DAV and EOI bits 1-4 rotated into bits 4-7
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

  // slow due to messy port layout but infrequently needed

  if (mask & 0b11100001) {

    // PORTC - REN bit 5 rotated into bit 6
    uint8_t portCb = (bits & 0x20) << 1;
    uint8_t portCm = (mask & 0x20) << 1;
    // PORTD - IFC bit 0 rotated into bit 4 and ATN bit 7 rotated into 1
    uint8_t portDb = ((bits & 0x01) << 4) | ((bits & 0x80) >> 6);
    uint8_t portDm = ((mask & 0x01) << 4) | ((mask & 0x80) >> 6);
    // PORT E - SRQ bit 6  in bit 6
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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

volatile uint8_t atnPinMem = ATNPREG;
volatile uint8_t srqPinMem = SRQPREG;
static const uint8_t ATNint = 0b00000010;
static const uint8_t SRQint = 0b01000000;

void pin_change_interrupt(void) {

  // Has the status of the ATN pin interrupt changed?
  if ((ATNPREG ^ atnPinMem) & ATNint) {
    // Set the current status of ATN
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has the status of the SRQ pin interrupt changed?
  if ((SRQPREG ^ srqPinMem) & SRQint) {
    // Set the current status of SRQ
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of the interrupt registers
  atnPinMem = ATNPREG;
  srqPinMem = SRQPREG;
}

void interruptsEn(){
  cli();
  attachInterrupt(digitalPinToInterrupt(ATN), pin_change_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SRQ), pin_change_interrupt, CHANGE);
  sei();  sei();
}

#endif  // USE_INTERRUPTS

#endif  // AR488_MEGA32U4_MICRO
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) BOARD LAYOUT for MICRO (Artag) *****/
/***********************************************************/



/************************************/
/***** LEONARDO R3 BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4_LR3

/***** Ready the GPIB data bus wires to receive data *****/
void readyGpibDbus() {
  // Set data pins to input

  DDRC &= 0b10111111 ;
  DDRD &= 0b11101111 ;
  DDRF &= 0b00001100 ;
 
  PORTC |= 0b01000000; // PORTD bit 4 input_pullup
  PORTD |= 0b00010000; // PORTD bit 6 input_pullup
  PORTF |= 0b11110011; // PORTC bits 7,6,5,4,1,0 input_pullup
}
/***** Collect a byte of data from the GPIB bus *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint8_t portf = reverseBits( (PINF & 0b11110000) + ((PINF & 0b00000011) << 2) );
  return ~( ((PIND & 0b00010000) << 2) + ((PINC & 0b01000000) <<1) + portf );
}


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
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
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
    mode (mode)     : 0=set pin state, 1=set pin direction
   Arduino Leonardo R3 pin to Port/bit to direction/state byte map:
   IFC   8   PORTB bit 4 byte bit 0
   NDAC  9   PORTB bit 5 byte bit 1
   NRFD  10  PORTB bit 6 byte bit 2
   DAV   11  PORTB bit 7 byte bit 3
   EOI   12  PORTD bit 6 byte bit 4
   SRQ   2   PORTD bit 1 byte bit 6
   REN   3   PORTD bit 0 byte bit 5
   ATN   7   PORTE bit 6 byte bit 7
*/
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // PORTB - use bits 0 to 3, rotate bits 4 positions left to set bits 4-7 on register (pins 8-12)
  uint8_t portBb = ((bits & 0x0F) << 4);
  uint8_t portBm = ((mask & 0x0F) << 4);
  // PORTD - use bit 4, rotate left 2 positions to set bit 6 on register (EOI)
  // PORTD - use bit 5, rotate right 5 positions to set bit 0 on register (REN)
  // PORTD - use bit 6, rotate right 5 positions to set bit 1 on register (SRQ)
  uint8_t portDb = ((bits & 0x10) << 2) + ((bits & 0x20) >> 5) + ((bits & 0x40) >> 5);
  uint8_t portDm = ((mask & 0x10) << 2) + ((mask & 0x20) >> 5) + ((mask & 0x40) >> 5);
  // PORTE - use bit 7, rotate left 1 position to set bit 6 on register (ATN)
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


/***** Enable interrupts *****/
#ifdef USE_INTERRUPTS

//volatile uint8_t atnPinMem = ATNPREG;
//volatile uint8_t srqPinMem = SRQPREG;
//static const uint8_t ATNint = 0b10000000;
//static const uint8_t SRQint = 0b00000100;

void atnISR() {
  isATN = (digitalRead(ATN) ? false : true);  
}

void srqISR() {
  isSRQ = (digitalRead(SRQ) ? false : true);  
}

void interruptsEn(){
  attachInterrupt(digitalPinToInterrupt(ATN), atnISR, CHANGE)
  attachInterrupt(digitalPinToInterrupt(SRQ), srqISR, CHANGE)
}

#endif //USE_INTERRUPTS


#endif //AR488_MEGA32U4_LR3
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** LEONARDO R3 BOARD LAYOUT *****/
/************************************/




/*************************************/
/***** CUSTOM PIN LAYOUT SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_CUSTOM

uint8_t databus[8] = { DIO1, DIO2, DIO3, DIO4, DIO5, DIO6, DIO7, DIO8 };

uint8_t ctrlbus[8] = { IFC, NDAC, NRFD, DAV, EOI, REN, SRQ, ATN };


/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
void readyGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], INPUT_PULLUP);
  }
}

uint8_t readGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    db = db + (digitalRead(databus[i]) ? 0 : 1<<i );
  }
  return db;
}


/***** Set the status of the GPIB data bus wires with a byte of datacd ~/test *****/
void setGpibDbus(uint8_t db) {

  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], OUTPUT);
    digitalWrite(databus[i], ((db&(1<<i)) ? LOW : HIGH) );
  }
  
}


/***** Set the direction and state of the GPIB control lines ****/
/*
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
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
