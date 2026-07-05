#ifndef M2560_H
#define M2560_H

#include "Arduino.h"
#include "../../../AR488_Config.h"



/************************************************/
/***** MEGA2560 LAYOUT DEFINITION (Default) *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_DF

// NOTE: MEGA2560 pinout last updated 28/07/2019
#define DIO1_PIN  A0  /* GPIB 1  : PORTF bit 0 */
#define DIO2_PIN  A1  /* GPIB 2  : PORTF bit 1 */
#define DIO3_PIN  A2  /* GPIB 3  : PORTF bit 2 */
#define DIO4_PIN  A3  /* GPIB 4  : PORTF bit 3 */
#define DIO5_PIN  A4  /* GPIB 13 : PORTF bit 4 */
#define DIO6_PIN  A5  /* GPIB 14 : PORTF bit 5 */
#define DIO7_PIN  A6  /* GPIB 15 : PORTF bit 4 */
#define DIO8_PIN  A7  /* GPIB 16 : PORTF bit 5 */

#define IFC_PIN   17  /* GPIB 9  : PORTH bit 0 */
#define NDAC_PIN  16  /* GPIB 8  : PORTH bit 1 */
#define NRFD_PIN   6  /* GPIB 7  : PORTH bit 3 */
#define DAV_PIN    7  /* GPIB 6  : PORTH bit 4 */
#define EOI_PIN    8  /* GPIB 5  : PORTH bit 5 */
#define REN_PIN    9  /* GPIB 17 : PORTD bit 6 */

#define SRQ_PIN   10  /* GPIB 10 : PORTB bit 4 */
#define ATN_PIN   11  /* GPIB 11 : PORTB bit 5 */

#endif  // AVR_MEGA2560_DF
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 LAYOUT DEFINITION (Default) *****/
/************************************************/



/*****************************************/
/***** MEGA2560 LAYOUT DEFINITION E1 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_E1

// NOTE: MEGA2560 pinout last updated 28/07/2019
#define DIO1_PIN  30  /* GPIB 1  : PORTC bit 1 */
#define DIO2_PIN  32  /* GPIB 2  : PORTC bit 3 */
#define DIO3_PIN  34  /* GPIB 3  : PORTC bit 5 */
#define DIO4_PIN  36  /* GPIB 4  : PORTC bit 7 */
#define DIO5_PIN  22  /* GPIB 13 : PORTA bit 6 */
#define DIO6_PIN  24  /* GPIB 14 : PORTA bit 4 */
#define DIO7_PIN  26  /* GPIB 15 : PORTA bit 2 */
#define DIO8_PIN  28  /* GPIB 16 : PORTA bit 0 */

#define IFC_PIN   48  /* GPIB 9  : PORTD bit 7 */
#define NDAC_PIN  46  /* GPIB 8  : PORTG bit 1 */
#define NRFD_PIN  44  /* GPIB 7  : PORTL bit 7 */
#define DAV_PIN   42  /* GPIB 6  : PORTL bit 5 */
#define EOI_PIN   40  /* GPIB 5  : PORTL bit 3 */
#define REN_PIN   38  /* GPIB 17 : PORTL bit 1 */

#define SRQ_PIN   50  /* GPIB 10 : PORTB bit 1 */
#define ATN_PIN   52  /* GPIB 11 : PORTB bit 3 */

#endif  // AVR_MEGA2560_E1
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 LAYOUT DEFINITION E1 *****/
/*****************************************/



/*****************************************/
/***** MEGA2560 LAYOUT DEFINITION E2 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA2560_E2

// NOTE: MEGA2560 pinout last updated 28/07/2019
#define DIO1_PIN  37  /* GPIB 1  : PORTA bit 1 */
#define DIO2_PIN  35  /* GPIB 2  : PORTA bit 3 */
#define DIO3_PIN  33  /* GPIB 3  : PORTA bit 5 */
#define DIO4_PIN  31  /* GPIB 4  : PORTA bit 7 */
#define DIO5_PIN  29  /* GPIB 13 : PORTC bit 6 */
#define DIO6_PIN  27  /* GPIB 14 : PORTC bit 4 */
#define DIO7_PIN  25  /* GPIB 15 : PORTC bit 2 */
#define DIO8_PIN  23  /* GPIB 16 : PORTC bit 0 */

#define IFC_PIN   49  /* GPIB 9  : PORTG bit 0 */
#define NDAC_PIN  47  /* GPIB 8  : PORTG bit 2 */
#define NRFD_PIN  45  /* GPIB 7  : PORTL bit 6 */
#define DAV_PIN   43  /* GPIB 6  : PORTL bit 4 */
#define EOI_PIN   41  /* GPIB 5  : PORTL bit 2 */
#define REN_PIN   39  /* GPIB 17 : PORTL bit 0 */

#define SRQ_PIN   51  /* GPIB 10 : PORTB bit 0 */
#define ATN_PIN   53  /* GPIB 11 : PORTB bit 2 */

#endif  // AVR_MEGA2560_E2
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MEGA2560 LAYOUT DEFINITION E2 *****/
/*****************************************/


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);


#endif  // M2560_H
