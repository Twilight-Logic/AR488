// specifics for arduino mega build (2560)

// version suffix
#define SUFFIX "mega 2560"

// Bluetooth enable
#define AR_BT_EN_PIN  12


// NOTE: Pinout last updated 28/07/2019
#define DIO1  A0  /* GPIB 1  : PORTF bit 0 */
#define DIO2  A1  /* GPIB 2  : PORTF bit 1 */
#define DIO3  A2  /* GPIB 3  : PORTF bit 2 */
#define DIO4  A3  /* GPIB 4  : PORTF bit 3 */
#define DIO5  A4  /* GPIB 13 : PORTF bit 4 */
#define DIO6  A5  /* GPIB 14 : PORTF bit 5 */
#define DIO7  A6  /* GPIB 15 : PORTF bit 4 */
#define DIO8  A7  /* GPIB 16 : PORTF bit 5 */

#define IFC   17  /* GPIB 9  : PORTH bit 0 */
#define NDAC  16  /* GPIB 8  : PORTH bit 1 */
#define NRFD  6   /* GPIB 7  : PORTH bit 3 */
#define DAV   7   /* GPIB 6  : PORTH bit 4 */
#define EOI   8   /* GPIB 5  : PORTH bit 5 */
#define REN   9   /* GPIB 17 : PORTD bit 6 */

#define SRQ  10   /* GPIB 10 : PORTB bit 4 */
#define ATN  11   /* GPIB 11 : PORTB bit 5 */


/*
   PIN interrupts
*/
#define ATNint 0b00100000
#define SRQint 0b00010000

// bit in PCICR that enables pin change interrupt for ATN and SRQ
#define PCICR_BIT 0b00000001;  // PORTB for mega

// copy of the interrupt pin reg to check for interrupt source
#define INTPINREG  PINB

// pin change vector
#define PCINT_vect PCINT0_vect 

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
//  DDRD &= 0b11001111 ;
//  DDRC &= 0b11000000 ;
  DDRF &= 0b00000000 ;

//  PORTD |= 0b00110000; // PORTD bits 5,4 input_pullup
//  PORTC |= 0b00111111; // PORTC bits 5,4,3,2,1,0 input_pullup
  PORTF |= 0b11111111; // set PORTC bits to input_pullup

  // Read the byte of data on the bus
  return ~(PINF & 0b11111111);
}


/*
   Set the status of the GPIB data bus wires with a byte of datacd ~/test

*/
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


/*
   Set the direction and state of the GPIB control lines
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


// LED pin
#define LED_MASK 0b10000000
#define LED_DDR  DDRB
#define LED_PORT PORTB
