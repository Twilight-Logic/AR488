#ifndef M4809_H
#define M4809_H

#include "Arduino.h"
#include "../../../AR488_Config.h"

/**************************************/
/***** KOFEN_POE_ETHERNET_ADAPTOR *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef KOFEN_POE_ETHERNET_ADAPTOR

/***** KOFEN's POE Ethernet Gpib Adaptor pinout *****/
#define DIO1_PIN  22  /* GPIB 1  : PORTC bit 0 */
#define DIO2_PIN  23  /* GPIB 2  : PORTC bit 1 */
#define DIO3_PIN  24  /* GPIB 3  : PORTC bit 2 */
#define DIO4_PIN  25  /* GPIB 4  : PORTC bit 3 */
#define DIO5_PIN  26  /* GPIB 13 : PORTC bit 4 */
#define DIO6_PIN  27  /* GPIB 14 : PORTC bit 5 */
#define DIO7_PIN  28  /* GPIB 15 : PORTD bit 4 */
#define DIO8_PIN  29  /* GPIB 16 : PORTD bit 5 */

#define IFC_PIN   18  /* GPIB 9  : PORTD bit 5 */
#define NDAC_PIN  17  /* GPIB 8  : PORTD bit 6 */
#define NRFD_PIN  16  /* GPIB 7  : PORTD bit 7 */
#define DAV_PIN   15  /* GPIB 6  : PORTB bit 0 */
#define EOI_PIN   14  /* GPIB 5  : PORTB bit 1 */
#define SRQ_PIN   19  /* GPIB 10 : PORTD bit 3 */
#define REN_PIN   21  /* GPIB 17 : PORTD bit 2 */
#define ATN_PIN   20  /* GPIB 11 : PORTD bit 4 */


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);
uint8_t readPortPullupReg(PORT_t& port);
void setPortPullupBits(PORT_t& port, uint8_t reg);


#endif  // KOFEN_POE_ETHERNET_ADAPTOR
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** KOFEN_POE_ETHERNET_ADAPTOR *****/
/**************************************/



#endif  // M4809.h
