#ifndef TEENSY41_H
#define TEENSY41_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/****************************************/
/***** TEENSY 4.1 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef IMXRT1062_TEENSY41_01

//#define TEENSY_ARDUINO_FUNCS
#define TEENSY_NATIVE_FUNCS

/***** NOTE: Teensy 4.1 board *****/
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

#define PIN_PULLUP_ENABLE   IOMUXC_PAD_PUS(3) | IOMUXC_PAD_PUE | IOMUXC_PAD_PKE | IOMUXC_PAD_DSE(6)
#define PIN_PULLUP_DISABLE  ~IOMUXC_PAD_PUE & ~IOMUXC_PAD_PKE
#define PIN_SION_ENABLE  (1UL<<4)
#define PIN_SION_DISABLE  ~(1UL<<4)

/*
 * PUS - pull-up/down strength 0=100k pull-down, 3=22k pull-up
 * PUE - pull-up enable
 * PKE - pull keeper enable
 * DSE - drive strength enable 1-7=impedance level, (150, 75, 50, 37, 30, 25, 20 ohms)
 * HYS - hysteresis
 */


#endif  // IMXRT1062_TEENSY41_01 
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** TEENSY 4.1 LAYOUT DEFINITION *****/
/****************************************/

void initTsyGpioPins();
void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);

#endif  // TEENSY41_H
