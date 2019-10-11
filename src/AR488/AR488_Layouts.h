#ifndef AR488_LAYOUTS_H
#define AR488_LAYOUTS_H

#include <Arduino.h>

#include "AR488_Config.h"

/***** AR488_Hardware.h, ver. 0.47.44, 09/10/2019 *****/


/*************************************/
/***** CUSTOM PIN LAYOUT SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_CUSTOM

// Use only pinhooks for custom mode
// (We don't know which pin interrupts will be required)
#ifdef USE_INTERRUPTS
  #undef USE_INTERRUPTS
  #define USE_PINHOOKS
#endif

#endif
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** CUSTOM PIN LAYOUT SECTION *****/
/*************************************/



/***************************************/
/***** UNO/NANO DEFINITION SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#if defined(AR488_UNO) || defined(AR488_NANO)


/***** NOTE: UNO/NANO pinout last updated 21/09/2019 *****/
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

/***** PIN interrupts ******/

// Interrupt registers
#ifdef USE_INTERRUPTS
  #define ATNPREG PIND
  #define SRQPREG PIND
  void interruptsEn();
#endif

#endif
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** UNO/NANO DEFINITION SECTION *****/
/***************************************/



/***************************************/
/***** MEGA2560 DEFINITION SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA2560

// NOTE: MEGA2560 pinout last updated 28/07/2019
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

// PCINT registers
#ifdef USE_INTERRUPTS
  #define ATNPREG PINB
  #define SRQPREG PINB
  void interruptsEn();
#endif  // USE_INTERRUPTS

#endif  // AR488_MEGA2560
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 DEFINITION SECTION *****/
/***************************************/



/***********************************************/
/***** MICRO PRO (32u4) DEFINITION SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_MEGA32U4

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

#ifdef USE_INTERRUPTS
  #define ATNPREG PIND
  #define SRQPREG PINE
  void interruptsEn();
#endif  // USE_INTERRUPTS

#endif  // AR488_32U4
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) DEFINITION SECTION *****/
/***********************************************/



/**************************************/
/***** GLOBAL DEFINITIONS SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvv *****/

uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibState(uint8_t bits, uint8_t mask, uint8_t mode);

/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GLOBAL DEFINITIONS SECTION *****/
/**************************************/


#endif // AR488_LAYOUTS_H
