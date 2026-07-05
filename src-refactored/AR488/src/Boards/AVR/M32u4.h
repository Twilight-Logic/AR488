#ifndef M32u4_H
#define M32u4_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/****************************************************************/
/***** MICRO PRO (32u4) LAYOUT DEFINITION for MICRO (Artag) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_32U4_MICRO

#define DIO1_PIN  3   /* GPIB 1  : PORTD bit 0   data pins assigned for minimum shifting */
#define DIO2_PIN  15  /* GPIB 2  : PORTB bit 1 */
#define DIO3_PIN  16  /* GPIB 3  : PORTB bit 2 */
#define DIO4_PIN  14  /* GPIB 4  : PORTB bit 3 */
#define DIO5_PIN  8   /* GPIB 13 : PORTB bit 4 */
#define DIO6_PIN  9   /* GPIB 14 : PORTB bit 5 */
#define DIO7_PIN  10  /* GPIB 15 : PORTB bit 6 */
#define DIO8_PIN  6   /* GPIB 16 : PORTD bit 7 */

#define IFC_PIN   4   /* GPIB 9  : PORTD bit 4 */
#define NDAC_PIN  A3  /* GPIB 8  : PORTF bit 4 */
#define NRFD_PIN  A2  /* GPIB 7  : PORTF bit 5 */
#define DAV_PIN   A1  /* GPIB 6  : PORTF bit 6 */
#define EOI_PIN   A0  /* GPIB 5  : PORTF bit 7 */
#define REN_PIN   5   /* GPIB 17 : PORTC bit 6 */
#define SRQ_PIN   7   /* GPIB 10 : PORTE bit 6 */
#define ATN_PIN   2   /* GPIB 11 : PORTD bit 1 */

#endif  // AVR_32U4_MICRO
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MICRO PRO (32u4) LAYOUT DEFINITION for MICRO (Artag) *****/
/****************************************************************/



/*****************************************/
/***** LEONARDO R3 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_32U4_LEO_R3

#define DIO1_PIN  A0  /* GPIB 1  : PORTF bit 7 */
#define DIO2_PIN  A1  /* GPIB 2  : PORTF bit 6 */
#define DIO3_PIN  A2  /* GPIB 3  : PORTF bit 5 */
#define DIO4_PIN  A3  /* GPIB 4  : PORTF bit 4 */
#define DIO5_PIN  A4  /* GPIB 13 : PORTF bit 3 */
#define DIO6_PIN  A5  /* GPIB 14 : PORTF bit 2 */
#define DIO7_PIN   4  /* GPIB 15 : PORTD bit 4 */
#define DIO8_PIN   5  /* GPIB 16 : PORTC bit 6 */

#define IFC_PIN    8  /* GPIB 9  : PORTB bit 4 */
#define NDAC_PIN   9  /* GPIB 8  : PORTB bit 5 */
#define NRFD_PIN  10  /* GPIB 7  : PORTB bit 6 */
#define DAV_PIN   11  /* GPIB 6  : PORTD bit 6 */
#define EOI_PIN   12  /* GPIB 5  : PORTC bit 7 */

#define SRQ_PIN    2  /* GPIB 10 : PORTD bit 1 */
#define REN_PIN    3  /* GPIB 17 : PORTD bit 0 */
#define ATN_PIN    7  /* GPIB 11 : PORTE bit 6 */

uint8_t reverseBits(uint8_t dbyte);

#endif // AVR_32U4_LEO_R3
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** LEONARDO R3 LAYOUT DEFINITION *****/
/*****************************************/


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);


#endif  // M32u4.h
