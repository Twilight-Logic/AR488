// specifics for arduino uno build (328)

// version suffix
#define SUFFIX "uno 328"

// Bluetooth enable
#define AR_BT_EN_PIN  6

// NOTE: Pinout last updated 09/01/2019
#define DIO1  A0  /* GPIB 1  : PORTC bit 0 */
#define DIO2  A1  /* GPIB 2  : PORTC bit 1 */
#define DIO3  A2  /* GPIB 3  : PORTC bit 2 */
#define DIO4  A3  /* GPIB 4  : PORTC bit 3 */
#define DIO5  A4  /* GPIB 13 : PORTC bit 4 */
#define DIO6  A5  /* GPIB 14 : PORTC bit 5 */
#define DIO7  4   /* GPIB 15 : PORTD bit 4 */
#define DIO8  5   /* GPIB 16 : PORTD bit 5 */

#define IFC   8   /* GPIB 9  : PORTB bit 0 */
#define NDAC  9   /* GPIB 8  : PORTB bit 1 */
#define NRFD  10  /* GPIB 7  : PORTB bit 2 */
#define DAV   11  /* GPIB 6  : PORTB bit 3 */
#define EOI   12  /* GPIB 5  : PORTB bit 4 */

#define SRQ   2   /* GPIB 10 : PORTD bit 2 */
#define REN   3   /* GPIB 17 : PORTD bit 3 */
#define ATN   7   /* GPIB 11 : PORTD bit 7 */


/*
   PIN interrupts
*/
#define ATNint 0b10000000
#define SRQint 0b00000100

// bit in PCICR that enables pin change interrupt for ATN and SRQ
#define PCICR_BIT 0b00000100;  // PORTD for uno/nano
//#define PCICR_BIT 0b00000001;  // PORTB for mega

// copy of the interrupt pin reg to check for interrupt source
#define INTPINREG  PIND

// pin change vector
#define PCINT_vect PCINT2_vect  // uno
//#define PCINT_vect PCINT0_vect  // mega

// pin change mask
#define PCMASK     PCMSK2

void setup_interrupts(void)
{
  cli();
  //  PCICR |= 0b00000001;  // PORTB
  //  PCICR |= 0b00000100;  // PORTD
  PCICR |= PCICR_BIT;
  sei();
}


/*
   Read the status of the GPIB data bus wires and collect the byte of data
*/
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


/*
   Set the status of the GPIB data bus wires with a byte of datacd ~/test

*/
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


/*
   Set the direction and state of the GPIB control lines
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


// LED pin
#define LED_MASK 0b00100000
#define LED_DDR  DDRB
#define LED_PORT PORTB
