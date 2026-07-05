#ifndef RA4M1_H
#define RA4M1_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/*************************************/
/***** NANO R4 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RA4M1_NANO_R4

#define DIO1_PIN  14  /* GPIB 1  : PORTC bit 0 */
#define DIO2_PIN  15  /* GPIB 2  : PORTC bit 1 */
#define DIO3_PIN  16  /* GPIB 3  : PORTC bit 2 */
#define DIO4_PIN  17  /* GPIB 4  : PORTC bit 3 */
#define DIO5_PIN  18  /* GPIB 13 : PORTC bit 4 */
#define DIO6_PIN  19  /* GPIB 14 : PORTC bit 5 */
#define DIO7_PIN   4  /* GPIB 15 : PORTD bit 4 */
#define DIO8_PIN   5  /* GPIB 16 : PORTD bit 5 */

#define IFC_PIN    8  /* GPIB 9  : PORTB bit 0 */
#define NDAC_PIN   9  /* GPIB 8  : PORTB bit 1 */
#define NRFD_PIN  10  /* GPIB 7  : PORTB bit 2 */
#define DAV_PIN   11  /* GPIB 6  : PORTB bit 3 */
#define EOI_PIN   12  /* GPIB 5  : PORTB bit 4 */

#define SRQ_PIN    2  /* GPIB 10 : PORTD bit 2 */
#define REN_PIN    3  /* GPIB 17 : PORTD bit 3 */
#define ATN_PIN    7  /* GPIB 11 : PORTD bit 7 */

#endif  // RA4M1_NANO_R4
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** NANO R4 LAYOUT DEFINITION *****/
/*************************************/


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);

#endif  // RA4M1_H
