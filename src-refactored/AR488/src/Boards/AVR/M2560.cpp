#include "M2560.h"


/*******************************************/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_DF
/*
  Data pin map
  ------------
  DIO1_PIN  A0 : GPIB 1  : PF0
  DIO2_PIN  A1 : GPIB 2  : PF1
  DIO3_PIN  A2 : GPIB 3  : PF2
  DIO4_PIN  A3 : GPIB 4  : PF3
  DIO5_PIN  A4 : GPIB 13 : PF4
  DIO6_PIN  A5 : GPIB 14 : PF5
  DIO7_PIN  A6 : GPIB 15 : PF6
  DIO8_PIN  A7 : GPIB 16 : PF7

  Control pin map
  ---------------
  IFC_PIN   17 : GPIB  9 : PB0 : b0
  NDAC_PIN  16 : GPIB  8 : PB1 : b1
  NRFD_PIN   6 : GPIB  7 : PB2 : b2
  DAV_PIN    7 : GPIB  6 : PB3 : b3
  EOI_PIN    8 : GPIB  5 : PB4 : b4
  REN_PIN   10 : GPIB 17 : PD2 : b5
  SRQ_PIN    9 : GPIB 10 : PD2 : b6
  ATN_PIN   11 : GPIB 11 : PD7 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/

/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRF |= 0b00000000;
    PORTF |= 0b11111111; // set PORTF bits to high
    return;
  }

  DDRF &= 0b00000000 ;
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


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  // PORT H - keep bits 5-0. Move bits 5-2 left 1 position to set bits 6-3 and 1-0 on port
  uint8_t portHb = ((bits & 0x3C) << 1) + (bits & 0x03);
  uint8_t portHm = ((mask & 0x3C) << 1) + (mask & 0x03);

  // PORT B - keep bits 7 and 6, but rotate right 2 postions to set bits 5 and 4 on port 
  uint8_t portBb = ((bits & 0xC0) >> 2);
  uint8_t portBm = ((mask & 0xC0) >> 2);
 
  // Set pin states using mask
  PORTH = ( (PORTH & ~portHm) | (portHb & portHm) );
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  // PORT H - keep bits 5-0. Move bits 5-2 left 1 position to set bits 6-3 and 1-0 on port
  uint8_t portHb = ((bits & 0x3C) << 1) + (bits & 0x03);
  uint8_t portHm = ((mask & 0x3C) << 1) + (mask & 0x03);

  // PORT B - keep bits 7 and 6, but rotate right 2 postions to set bits 5 and 4 on port 
  uint8_t portBb = ((bits & 0xC0) >> 2);
  uint8_t portBm = ((mask & 0xC0) >> 2);
 
  // Set registers: register = (register & ~bitmask) | (value & bitmask)
  // Mask: 0=unaffected; 1=to be changed

  // Set pin direction registers using mask
  DDRH = ( (DDRH & ~portHm) | (portHb & portHm) );
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // AVR_MEGA2560_DF
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT (Default) *****/
/*******************************************/



/************************************/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_E1
/*
  Data pin map
  ------------
  DIO1_PIN  30 : GPIB 1  : PC1
  DIO2_PIN  32 : GPIB 2  : PC3
  DIO3_PIN  34 : GPIB 3  : PC5
  DIO4_PIN  36 : GPIB 4  : PC7
  DIO5_PIN  22 : GPIB 13 : PA6
  DIO6_PIN  24 : GPIB 14 : PA4
  DIO7_PIN  26 : GPIB 15 : PA2
  DIO8_PIN  28 : GPIB 16 : PA0

  Control pin map
  ---------------
  IFC_PIN   48 : GPIB  9 : PL1 : b0
  NDAC_PIN  48 : GPIB  8 : PL3 : b1
  NRFD_PIN  44 : GPIB  7 : PL5 : b2
  DAV_PIN   42 : GPIB  6 : PL7 : b3
  EOI_PIN   40 : GPIB  5 : PG1 : b4
  REN_PIN   50 : GPIB 17 : PB3 : b5
  SRQ_PIN   38 : GPIB 10 : PD7 : b6
  ATN_PIN   52 : GPIB 11 : PB1 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRA |= 0b01010101;
    DDRC |= 0b10101010;
    PORTA |= 0b01010101; // PORTA bits 6,4,2,0 high
    PORTC |= 0b10101010; // PORTC bits 7,5,3,1 high
    return;
  }

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


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

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

  // Set pin states using mask
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
  PORTD = ( (PORTD & ~portDm) | (portDb & portDm) );
  PORTG = ( (PORTG & ~portGm) | (portGb & portGm) );
  PORTL = ( (PORTL & ~portLm) | (portLb & portLm) );
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

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

  // Set pin direction registers using mask
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
  DDRD = ( (DDRD & ~portDm) | (portDb & portDm) );
  DDRG = ( (DDRG & ~portGm) | (portGb & portGm) );
  DDRL = ( (DDRL & ~portLm) | (portLb & portLm) );
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // AVR_MEGA2560_E1
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E1 *****/
/************************************/



/************************************/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_E2
/*
  Data pin map
  ------------
  DIO1_PIN  37 : GPIB 1  : PA1
  DIO2_PIN  35 : GPIB 2  : PA3
  DIO3_PIN  33 : GPIB 3  : PA5
  DIO4_PIN  31 : GPIB 4  : PA7
  DIO5_PIN  29 : GPIB 13 : PC6
  DIO6_PIN  27 : GPIB 14 : PC4
  DIO7_PIN  25 : GPIB 15 : PC2
  DIO8_PIN  23 : GPIB 16 : PC0

  Control pin map
  ---------------
  IFC_PIN   49 : GPIB  9 : PG0 : b0
  NDAC_PIN  47 : GPIB  8 : PG2 : b1
  NRFD_PIN  45 : GPIB  7 : PL6 : b2
  DAV_PIN   43 : GPIB  6 : PL4 : b3
  EOI_PIN   41 : GPIB  5 : PL2 : b4
  REN_PIN   51 : GPIB 17 : PB0 : b5
  SRQ_PIN   39 : GPIB 10 : PL0 : b6
  ATN_PIN   53 : GPIB 11 : PB2 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    // Set data pins to output
    DDRA |= 0b10101010;
    DDRA |= 0b01010101;
    PORTA |= 0b10101010; // PORTC bits 7,5,3,1 high
    PORTC |= 0b01010101; // PORTA bits 6,4,2,0 high
    return;
  }
  
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


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

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

  // Set pin states using mask
  PORTB = ( (PORTB & ~portBm) | (portBb & portBm) );
  PORTG = ( (PORTG & ~portGm) | (portGb & portGm) );
  PORTL = ( (PORTL & ~portLm) | (portLb & portLm) );
}



void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

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

  // Set pin direction registers using mask
  DDRB = ( (DDRB & ~portBm) | (portBb & portBm) );
  DDRG = ( (DDRG & ~portGm) | (portGb & portGm) );
  DDRL = ( (DDRL & ~portLm) | (portLb & portLm) );
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // AVR_MEGA2560_E2
/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 BOARD LAYOUT E2 *****/
/************************************/


