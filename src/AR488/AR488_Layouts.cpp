#include <Arduino.h>

#include "AR488_Config.h"
#include "AR488_Layouts.h"

/***** AR488_Hardware.cpp, ver. 0.47.44, 09/10/2019 *****/


volatile bool isATN = false;  // has ATN been asserted?
volatile bool isSRQ = false;  // has SRQ been asserted?


/**********************************/
/***** UNO/NANO BOARD SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvv *****/
#if defined(AR488_UNO) || defined(AR488_NANO)

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
uint8_t readGpibDbus() {
  // Set data pins to input
  //  DDRD = DDRD & 0b11001111 ;
  //  DDRC = DDRC & 0b11000000 ;
  DDRD &= 0b11001111 ;
  DDRC &= 0b11000000 ;
  //  PORTD = PORTD | 0b00110000; // PORTD bits 5,4 input_pullup
  //  PORTC = PORTC | 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup

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
   pdir:  0=input, 1=output;
   pstat: 0=LOW, 1=HIGH/INPUT_PULLUP
   Arduino pin to Port/bit to direction/state byte map:
   IFC   8   PORTB bit 0 byte bit 0
   NDAC  9   PORTB bit 1 byte bit 1
   NRFD  10  PORTB bit 2 byte bit 2
   DAV   11  PORTB bit 3 byte bit 3
   EOI   12  PORTB bit 4 byte bit 4
  // * REN   13  PORTB bit 5 byte bit 5
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
/***** ^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** UNO/NANO BOARD SECTION *****/
/**********************************/



/***************************************/
/***** MEGA2560 BOARD CODE SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560

/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
uint8_t readGpibDbus() {
  // Set data pins to input
//  DDRD &= 0b11001111 ;
//  DDRC &= 0b11000000 ;
  DDRF &= 0b00000000 ;

//  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
//  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTF |= 0b11111111; // set PORTC bits to input_pullup

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
   pdir:  0=input, 1=output;
   pstat: 0=LOW, 1=HIGH/INPUT_PULLUP
   Arduino pin to Port/bit to direction/state byte map:
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

/***** Catchall interrupt vector *****/
/*
  ISR(BADISR_vect) {
  // ISR to catch ISR firing without handler
  isBAD = true;
  }
*/
#endif //USE_INTERRUPTS


#endif //MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD CODE SECTION *****/
/***************************************/



/***********************************************/
/***** MICRO PRO (32u4) BOARD CODE SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4

uint8_t readGpibDbus() {

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
 * pdir:  0=input, 1=output;
 * pstat: 0=LOW, 1=HIGH/INPUT_PULLUP
 * Arduino pin to Port/bit to direction/state byte map:
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

  // Has ATN pin interrupt fired (ATN asserted)?
  if ((ATNPREG ^ atnPinMem) & ATNint) {
    isATN = (ATNPREG & ATNint) == 0;
  }

  // Has SRQ pin interrupt fired (SRQ asserted)?
  if ((SRQPREG ^ srqPinMem) & SRQint) {
    isSRQ = (SRQPREG & SRQint) == 0;
  }

  // Save current state of interrupt pin's register
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

#endif  // MEGA32U4
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) BOARD CODE SECTION *****/
/***********************************************/






/*************************************/
/***** CUSTOM PIN LAYOUT SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_CUSTOM

uint8_t databus[8] = { DIO1, DIO2, DIO3, DIO4, DIO5, DIO6, DIO7, DIO8 };

uint8_t ctrlbus[8] = { IFC, NDAC, NRFD, DAV, EOI, REN, SRQ, ATN };


/***** Read the status of the GPIB data bus wires and collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;

  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], INPUT_PULLUP);
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
