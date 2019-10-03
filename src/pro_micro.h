// specifics for arduino pro micro build (32u4)
// this is the version of the pro micro that's laid out the same as a pro mini (but has 32u4 pin assignments)
// pins assigned for reasonably efficient port mapping but ugly to wire unless you have a pcb or non-ribbon cable

// version suffix
#define SUFFIX "pro micro 32u4"

// Bluetooth enable
#define AR_BT_EN_PIN  NONE

// report pin changes in excruciating detail
//#define MICRODEBUG 1

// NOTE: Pinout last updated 16/09/2019
#define DIO1  3   /* GPIB 1  : PORTD bit 0   data pins assigned for minimum shifting */
#define DIO2  15  /* GPIB 2  : PORTB bit 1 */
#define DIO3  16  /* GPIB 3  : PORTB bit 2 */
#define DIO4  14  /* GPIB 4  : PORTB bit 3 */
#define DIO5  8   /* GPIB 13 : PORTB bit 4 */
#define DIO6  9   /* GPIB 14 : PORTB bit 5 */
#define DIO7  10  /* GPIB 15 : PORTB bit 6 */
#define DIO8  6   /* GPIB 16 : PORTD bit 7 */

#define IFC   4   /* GPIB 9  : PORTD bit 4 */
#define NDAC  A3  /* GPIB 8  : PORTF bit 4   fast control pins assigned to same port */
#define NRFD  A2  /* GPIB 7  : PORTF bit 5 */
#define DAV   A1  /* GPIB 6  : PORTF bit 6 */
#define EOI   A0  /* GPIB 5  : PORTF bit 7 */
#define REN   5   /* GPIB 17 : PORTC bit 6 */
#define SRQ   7   /* GPIB 10 : PORTE bit 6 */
#define ATN   2   /* GPIB 11 : PORTD bit 1 */


/*
   PIN interrupts (but beware, also used as pin masks which ought to be defined separately)
*/
#define ATNint 0b00000010   // EIMSK bits
#define SRQint 0b01000000

// copy of the interrupt pin reg to check for interrupt source
// this is a hack (to keep code fairly similar to Uno version) that only works because 
// although we look in two registers, the important bits are not in the same location
#define INTPINREG   ((PIND&ATNint)|(PINE&SRQint))

// pin change mask
// this is a hack that chooses external interrupts in a similar way to pinchange interrupts
#define PCMASK     EIMSK

extern void pin_change_interrupt(void);

void setup_interrupts(void)
{
  cli();
  attachInterrupt(digitalPinToInterrupt(ATN), pin_change_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SRQ), pin_change_interrupt, CHANGE);
  sei();
}

/*
   Read the status of the GPIB data bus wires and collect the byte of data
   port bits arec numbered 0..7 but gpib bits are numbered 1..8
*/

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


/*
   Set the status of the GPIB data bus wires with a byte of datacd ~/test

*/

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

/*
   Set the direction and state of the GPIB control lines
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
   pdir:  0=input, 1=output;
   pstat: 0=LOW, 1=HIGH/INPUT_PULLUP
   Arduino pin to Port/bit to direction/state byte map:
   IFC   4   PORTD bit 4   byte bit 0
   NDAC  A3  PORTF bit 4   byte bit 1
   NRFD  A2  PORTF bit 5   byte bit 2
   DAV   A1  PORTF bit 6   byte bit 3
   EOI   A0  PORTF bit 7   byte bit 4
   REN   5   PORTC bit 6   byte bit 5
   SRQ   7   PORTE bit 6   byte bit 6
   ATN   2   PORTD bit 1   byte bit 7


   It would be more efficient (and easier to read the code) if the bits in the above
   control word were assigned by name to match suitable port bits : then NDAC,NRFD anbd DAV
   could be positioned at bits 4,5,6 to be placed in port F without shifting.

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



// LED pin - no dedicated led, though the rx/tx are available as PD5/PB0
#define LED_MASK 0b00000000
#define LED_DDR  DDRB
#define LED_PORT PORTB





#define SRQ_53131A \
  /* do an SRQ-raising operation on 53131A  - page 3-48 */     \
  ":CALC3:AVERAGE ON\n"        /* set up a slow measurement */ \
  ":CALC3:AVERAGE:COUNT 50\n" \
  ":TRIG:COUNT:AUTO ON\n"     \
  "*ESE 1\n"                   /* enable measurement completion as event */  \
  "*SRE 32\n"                  /* generate SRQ on event */                   \
  ":INIT\n"                    /* start measurement */                       \
  "*OPC\n"                    \
  /* then do this when SRQ is raised */  \ 
  // ":CALC3:AVERAGE:ALL?\n"









#if 0

// specifics for arduino pro micro build (32u4)
// initial version had pins assigned for hopelessly inefficient port mapping but easy wiring
// useful if you hand-wire the micro pins to the microribbon connector

// version suffix
#define SUFFIX "pro micro 32u4 handwire"

// Bluetooth enable
#define AR_BT_EN_PIN  NONE

// report pin changes in excruciating detail
//#define MICRODEBUG 1

// NOTE: Pinout last updated 09/09/2019
#define DIO1  10  /* GPIB 1  : PORTB bit 6 */
#define DIO2  16  /* GPIB 2  : PORTB bit 2 */
#define DIO3  14  /* GPIB 3  : PORTB bit 3 */
#define DIO4  15  /* GPIB 4  : PORTB bit 1 */
#define DIO5  9   /* GPIB 13 : PORTB bit 5 */
#define DIO6  8   /* GPIB 14 : PORTB bit 4 */
#define DIO7  7   /* GPIB 15 : PORTE bit 6 */
#define DIO8  6   /* GPIB 16 : PORTD bit 7 */

#define IFC   4   /* GPIB 9  : PORTD bit 4 */
#define NDAC  A3  /* GPIB 8  : PORTF bit 4 */
#define NRFD  A2  /* GPIB 7  : PORTF bit 5 */
#define DAV   A1  /* GPIB 6  : PORTF bit 6 */
#define EOI   A0  /* GPIB 5  : PORTF bit 7 */

#define SRQ   3   /* GPIB 10 : PORTD bit 0 */
#define REN   5   /* GPIB 17 : PORTC bit 6 */
#define ATN   2   /* GPIB 11 : PORTD bit 1 */


/*
   PIN interrupts (but beware, also used as pin masks which ought to be defined separately)
*/
#define ATNint 0b00000010   // EIMSK bits
#define SRQint 0b00000001

// copy of the interrupt pin reg to check for interrupt source
// this is a hack (to keep code fairly similar to Uno version) that only works because 
// although we look in two registers, the important bits are not in the same location
#define INTPINREG   PIND

// pin change mask
// this is a hack that chooses external interrupts in a similar way to pinchange interrupts
#define PCMASK     EIMSK

extern void pin_change_interrupt(void);

void setup_interrupts(void)
{
  cli();
  attachInterrupt(digitalPinToInterrupt(ATN), pin_change_interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SRQ), pin_change_interrupt, CHANGE);
  sei();
}


/*
   Read the status of the GPIB data bus wires and collect the byte of data
   port bits arec numbered 0..7 but gpib bits are numbered 1..8
*/


// take 3 bits of the portB input and descramble to data (for 2 sets of 3 bits)
// faster to do all 6 bits together but that needs a 64-byte table for each direction
static const uint8_t dbdemangle321[] = {0, 0x08, 0x02, 0x0a, 0x04, 0x0c, 0x06, 0x0e }; // bits 321 -> gpib 324
static const uint8_t dbdemangle654[] = {0, 0x20, 0x10, 0x30, 0x01, 0x21, 0x11, 0x31 }; // bits 654 -> gpib 156

uint8_t readGpibDbus() {

  // Set data pins to input
  DDRB  &= 0b10000001 ;
  DDRE  &= 0b10111111 ;
  DDRD  &= 0b01111111 ;
  PORTB |= 0b01111110; // PORTB bits 6,5,4,3,2,1 input_pullup
  PORTE |= 0b01000000; // PORTE bit 6 input_pullup
  PORTD |= 0b10000000; // PORTD bit 7 input_pullup

  // Read the byte of data on the bus
  // DIO8 -> PORTD bit 7, DIO7 -> PORTE bit 5, DIO6-DIO1 -> PORTB bit 451326

#ifdef MICRODEBUG
  Serial.print("B ");
  Serial.print(PINB & 0x7e, HEX);
  Serial.print(", E ");
  Serial.print(PINE & 0x40, HEX);
  Serial.print(", D ");
  Serial.print(PIND & 0x80, HEX);

  uint8_t x = ~((PIND & 0b10000000) | (PINE & 0b01000000) | dbdemangle321[PINB >> 1 & 0b00000111] | dbdemangle654[PINB >> 4 & 0b00000111]);
  Serial.print(" value ");
  Serial.println(x);
#endif

  return ~((PIND & 0b10000000) | (PINE & 0b01000000) | dbdemangle321[PINB >> 1 & 0b00000111] | dbdemangle654[PINB >> 4 & 0b00000111]);
}


/*
   Set the status of the GPIB data bus wires with a byte of datacd ~/test

*/

// take 3 bits of gpib data and scramble into portB assignments (for two sets of 3 bits)
static const uint8_t dbmangle321[]   = {0, 0x40, 0x04, 0x44, 0x08, 0x48, 0x0c, 0x4c }; // bits 321 -> 326
static const uint8_t dbmangle654[]   = {0, 0x02, 0x20, 0x22, 0x10, 0x12, 0x30, 0x32 }; // bits 654 -> 451

void setGpibDbus(uint8_t db) {

  //Serial.print("dbus 0x");
  //Serial.println(db, HEX);

  // Set data pins as outputs
  DDRB |= 0b01111110;
  DDRE |= 0b01000000;
  DDRD |= 0b10000000;

  // GPIB states are inverted
  db = ~db;

  // Set data bus
  PORTB = (PORTB & ~0b01111110) | dbmangle321[(db & 0b00000111)] | dbmangle654[db >> 3 & 0b00000111] ;
  PORTE = (PORTE & ~0b01000000) | (db & 0b01000000);
  PORTD = (PORTD & ~0b10000000) | (db & 0b10000000);

#ifdef MICRODEBUG
  Serial.print("bits B ");
  Serial.print(dbmangle321[(db & 0b00000111)] | dbmangle654[db >> 3 & 0b00000111], HEX);
  Serial.print(", bits E ");
  Serial.print(db & 0b01000000, HEX);
  Serial.print(", bits D ");
  Serial.println(db & 0b10000000, HEX);
#endif
}

/*
   Set the direction and state of the GPIB control lines
   Bits control lines as follows: 7-ATN, 6-SRQ, 5-REN, 4-EOI, 3-DAV, 2-NRFD, 1-NDAC, 0-IFC
   pdir:  0=input, 1=output;
   pstat: 0=LOW, 1=HIGH/INPUT_PULLUP
   Arduino pin to Port/bit to direction/state byte map:
   IFC   4   PORTD bit 4   byte bit 0
   NDAC  A3  PORTF bit 4   byte bit 1
   NRFD  A2  PORTF bit 5   byte bit 2
   DAV   A1  PORTF bit 6   byte bit 3
   EOI   A0  PORTF bit 7   byte bit 4
   REN   5   PORTC bit 6   byte bit 5
   SRQ   3   PORTD bit 0   byte bit 6
   ATN   2   PORTD bit 1   byte bit 7

  See hardware/tools/avr/avr/include/avr/sfr_defs.h for detailsa of indirect port addresssing

*/

struct port {
  uint8_t port;
  uint8_t ddr;
  uint8_t mask;
};

static const struct port ports[] = {

  { _SFR_IO_ADDR(PORTD), _SFR_IO_ADDR(DDRD), 0x01 },
  { _SFR_IO_ADDR(PORTF), _SFR_IO_ADDR(DDRF), 0x10 },
  { _SFR_IO_ADDR(PORTF), _SFR_IO_ADDR(DDRF), 0x20 },
  { _SFR_IO_ADDR(PORTF), _SFR_IO_ADDR(DDRF), 0x40 },
  { _SFR_IO_ADDR(PORTF), _SFR_IO_ADDR(DDRF), 0x80 },
  { _SFR_IO_ADDR(PORTC), _SFR_IO_ADDR(DDRC), 0x40 },
  { _SFR_IO_ADDR(PORTD), _SFR_IO_ADDR(DDRD), 0x01 },
  { _SFR_IO_ADDR(PORTD), _SFR_IO_ADDR(DDRD), 0x02 },
};


void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode) {

  // change the bits serially. This is slow but most of the time only one changes.

  struct port *p = ports;

#ifdef MICRODEBUG
  Serial.print("setGpibState mode ");
  Serial.print(mode);
  Serial.print(" to ");
  Serial.print(bits, HEX);
  Serial.print(" mask ");
  Serial.println(mask, HEX);
#endif

  if (mode) {
    for (uint8_t bit = 0x1; mask; bit <<= 1, ++p) {
      if (mask & bit) {
        _SFR_IO8(p->ddr) = (_SFR_IO8(p->ddr) & ~p->mask) | (bits ? p->mask : 0);
        mask &= ~(bit);
      }
    }

  } else {

    for (uint8_t bit = 0x1; mask; bit <<= 1, ++p) {
      switch (mask) {
        // catch some common cases - the data handshake - first
        case 0x08: // DAV:
          return _SFR_IO8(p->port) = (_SFR_IO8(p->port) & ~p->mask) | (bits ? p->mask : 0);
        case 0x04: // NRFD:
          return _SFR_IO8(p->port) = (_SFR_IO8(p->port) & ~p->mask) | (bits ? p->mask : 0);
        case 0x02: // NDAC:
          return _SFR_IO8(p->port) = (_SFR_IO8(p->port) & ~p->mask) | (bits ? p->mask : 0);
        default:
          if (mask & bit) {
            _SFR_IO8(p->port) = (_SFR_IO8(p->port) & ~p->mask) | (bits ? p->mask : 0);
            mask &= ~(bit);
          }
          break;
      }
    }
  }
}

// LED pin - no dedicated led, though the rx/tx are available as PD5/PB0
#define LED_MASK 0b00000000
#define LED_DDR  DDRB
#define LED_PORT PORTB

#endif
