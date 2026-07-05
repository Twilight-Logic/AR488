#ifndef CUSTOM_H
#define CUSTOM_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/******************************************/
/***** AR488 CUSTOM LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
/*
 * This layout is the default UNO layout
 * but can be modified as required
 * to suit any unknown development board.
 * To use it, ensure that #define AR488_CUSTOM
 * is set in AR488_Config.h 
 */

#ifdef AR488_CUSTOM_LAYOUT


#define DIO1_PIN  23  /* GPIB 1  : AD_B1_09 : GPIO1_25 */
#define DIO2_PIN  22  /* GPIB 2  : AD_B1_08 : GPIO1_24 */
#define DIO3_PIN  21  /* GPIB 3  : AD_B1_11 : GPIO1_27 */
#define DIO4_PIN  20  /* GPIB 4  : AD_B1_10 : GPIO1_26 */
#define DIO5_PIN  19  /* GPIB 13 : AD_B1_00 : GPIO1_16 */
#define DIO6_PIN  18  /* GPIB 14 : AD_B1_01 : GPIO1_17 */
#define DIO7_PIN  17  /* GPIB 15 : AD_B1_06 : GPIO1_22 */
#define DIO8_PIN  16  /* GPIB 16 : AD_B1_07 : GPIO1_23 */

#define IFC_PIN   15  /* GPIB 9  : AD_B1_03 : GPIO1_19 */
#define NDAC_PIN  14  /* GPIB 8  : AD_B1_02 : GPIO1_18 */
#define NRFD_PIN  41  /* GPIB 7  : AD_B1_05 : GPIO1_21 */
#define DAV_PIN   40  /* GPIB 6  : AD_B1_04 : GPIO1_20 */
#define EOI_PIN   39  /* GPIB 5  : AD_B1_13 : GPIO1_29 */

#define SRQ_PIN   38  /* GPIB 10 : AD_B1_12 : GPIO1_28 */
#define REN_PIN   26  /* GPIB 17 : AD_B1_14 : GPIO1_30 */
#define ATN_PIN   27  /* GPIB 11 : AD_B1_15 : GPIO1_31 */



//#define DIO1_PIN  A0  /* GPIB 1  */
//#define DIO2_PIN  A1  /* GPIB 2  */
//#define DIO3_PIN  A2  /* GPIB 3  */
//#define DIO4_PIN  A3  /* GPIB 4  */
//#define DIO5_PIN  A4  /* GPIB 13 */
//#define DIO6_PIN  A5  /* GPIB 14 */
//#define DIO7_PIN  4   /* GPIB 15 */
//#define DIO8_PIN  5   /* GPIB 16 */

//#define IFC_PIN   8   /* GPIB 9  */
//#define NDAC_PIN  9   /* GPIB 8  */
//#define NRFD_PIN  10  /* GPIB 7  */
//#define DAV_PIN   11  /* GPIB 6  */
//#define EOI_PIN   12  /* GPIB 5  */

//#define SRQ_PIN   2   /* GPIB 10 */
//#define REN_PIN   3   /* GPIB 17 */
//#define ATN_PIN   7   /* GPIB 11 */

#endif
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** AR488 CUSTOM LAYOUT DEFINITION*****/
/*****************************************/


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);


#endif  // CUSTOM_H
