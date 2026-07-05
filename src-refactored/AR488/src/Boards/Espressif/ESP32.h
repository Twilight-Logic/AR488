#ifndef ESP32_H
#define ESP32_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/***********************************/
/***** ESP32 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_DEVKIT1_WROOM_32

// Pin numbers represent GPIOnum
// DC = REN
// TE = 2 (or 15)

//#define ESP32_ARDUINO_FUNC   // Standard Arduino functions
#define ESP32_NATIVE_FUNC      // ESP hybrid functions



#define DIO1_PIN  32   /* GPIB 1  */
#define DIO2_PIN  33   /* GPIB 2  */
#define DIO3_PIN  25   /* GPIB 3  */
#define DIO4_PIN  26   /* GPIB 4  */
#define DIO5_PIN  27   /* GPIB 13 */
#define DIO6_PIN  14   /* GPIB 14 */
#define DIO7_PIN   4   /* GPIB 15 */
#define DIO8_PIN  13   /* GPIB 16 */

#define IFC_PIN   23   /* GPIB 9  */
#define NDAC_PIN  22   /* GPIB 8  */
#define NRFD_PIN  21   /* GPIB 7  */
#define DAV_PIN   19   /* GPIB 6  */
#define EOI_PIN   18   /* GPIB 5  */

#define SRQ_PIN    5   /* GPIB 10 */
#define REN_PIN   17   /* GPIB 17 */
#define ATN_PIN   16   /* GPIB 11 */

#endif // ESP32_DEVKIT1_WROOM_32
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32 LAYOUT DEFINITION *****/
/***********************************/



/***********************************************/
/***** ESP32_TTGO_T8_161 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_TTGO_T8_161

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN  34   /* GPIB 1  */
#define DIO2_PIN  35   /* GPIB 2  */
#define DIO3_PIN  32   /* GPIB 3  */
#define DIO4_PIN  33   /* GPIB 4  */
#define DIO5_PIN  25   /* GPIB 13 */
#define DIO6_PIN  26   /* GPIB 14 */
#define DIO7_PIN  27   /* GPIB 15 */
#define DIO8_PIN  14   /* GPIB 16 */

#define IFC_PIN   15   /* GPIB 9  */
#define NDAC_PIN  22   /* GPIB 8  */
#define NRFD_PIN  19   /* GPIB 7  */
#define DAV_PIN   23   /* GPIB 6  */
#define EOI_PIN   18   /* GPIB 5  */

#define SRQ_PIN    2   /* GPIB 10 */
#define REN_PIN   13   /* GPIB 17 */
#define ATN_PIN    5   /* GPIB 11 */

#endif // ESP32_TTGO_T8_161
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32_TTGO_T8_161 LAYOUT DEFINITION *****/
/****************************************************/



/********************************************/
/***** ESP32_ESP32DEV LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_ESP32DEV

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN  33   /* GPIB 1  */
#define DIO2_PIN  32   /* GPIB 2  */
#define DIO3_PIN  26   /* GPIB 3  */
#define DIO4_PIN  25   /* GPIB 4  */
#define DIO5_PIN  14   /* GPIB 13 */
#define DIO6_PIN  27   /* GPIB 14 */
#define DIO7_PIN  13   /* GPIB 15 */
#define DIO8_PIN  12   /* GPIB 16 */

#define IFC_PIN   22   /* GPIB 9  */
#define NDAC_PIN  21   /* GPIB 8  */
#define NRFD_PIN  19   /* GPIB 7  */
#define DAV_PIN   18   /* GPIB 6  */
#define EOI_PIN   17   /* GPIB 5  */

#define SRQ_PIN   16   /* GPIB 10 */
#define REN_PIN   23   /* GPIB 17 */
#define ATN_PIN    4   /* GPIB 11 */

#endif // ESP32_ESP32DEV
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32_ESP32DEV LAYOUT DEFINITION *****/
/********************************************/



/***********************************************/
/***** ESP32_LOLIN32_161 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
/*
 * Note: V1 and V2 have the same pin assignments
 */
#ifdef ESP32_LOLIN32_161

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN  32   /* GPIB 1  */
#define DIO2_PIN  33   /* GPIB 2  */
#define DIO3_PIN  25   /* GPIB 3  */
#define DIO4_PIN  26   /* GPIB 4  */
#define DIO5_PIN  27   /* GPIB 13 */
#define DIO6_PIN  14   /* GPIB 14 */
#define DIO7_PIN  12   /* GPIB 15 */
#define DIO8_PIN  13   /* GPIB 16 */

#define IFC_PIN   21   /* GPIB 9  */
#define NDAC_PIN  19   /* GPIB 8  */
#define NRFD_PIN  23   /* GPIB 7  */
#define DAV_PIN   18   /* GPIB 6  */
#define EOI_PIN   17   /* GPIB 5  */

#define SRQ_PIN    4   /* GPIB 10 */
#define REN_PIN   22   /* GPIB 17 */
#define ATN_PIN   16   /* GPIB 11 */

#endif // ESP32_LOLIN32_161
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32_LOLIN32_161 LAYOUT DEFINITION *****/
/***********************************************/



/***********************************************/
/***** ESP32_S2_161 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_S2_161

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN   1   /* GPIB 1  */
#define DIO2_PIN   2   /* GPIB 2  */
#define DIO3_PIN   3   /* GPIB 3  */
#define DIO4_PIN   4   /* GPIB 4  */
#define DIO5_PIN   5   /* GPIB 13 */
#define DIO6_PIN   6   /* GPIB 14 */
#define DIO7_PIN   7   /* GPIB 15 */
#define DIO8_PIN   8   /* GPIB 16 */

#define IFC_PIN   10   /* GPIB 9  */
#define NDAC_PIN  11   /* GPIB 8  */
#define NRFD_PIN  12   /* GPIB 7  */
#define DAV_PIN   13   /* GPIB 6  */
#define EOI_PIN   14   /* GPIB 5  */

#define SRQ_PIN   16   /* GPIB 10 */
#define REN_PIN    9   /* GPIB 17 */
#define ATN_PIN   15   /* GPIB 11 */

#endif // ESP32_S2_161
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32_S2_161 LAYOUT DEFINITION *****/
/***********************************************/



/******************************************************/
/***** Wilhelm_AR488_ESP32S2_R4 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_Wilhelm_AR488_ESP32S2_R4

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN   8    /* GPIB  8 */
#define DIO2_PIN   9    /* GPIB  9 */
#define DIO3_PIN   10   /* GPIB 10 */
#define DIO4_PIN   11   /* GPIB 11 */
#define DIO5_PIN   12   /* GPIB 12 */
#define DIO6_PIN   13   /* GPIB 13 */
#define DIO7_PIN   14   /* GPIB 14 */
#define DIO8_PIN   15   /* GPIB 15 */

#define IFC_PIN    38   /* GPIB 34 */
#define NDAC_PIN   33   /* GPIB 33 */
#define NRFD_PIN   34   /* GPIB 34 */
#define DAV_PIN    40   /* GPIB 40 */
#define EOI_PIN    35   /* GPIB 35 */

#define SRQ_PIN    37   /* GPIB 37 */
#define REN_PIN    26   /* GPIB 21 */
#define ATN_PIN    36   /* GPIB 36 */

#endif // ESP32_Wilhelm_AR488_ESP32S2_R4
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** Wilhelm_AR488_ESP32S2_R4 LAYOUT DEFINITION *****/
/******************************************************/



/******************************************************/
/***** Wilhelm_AR488_ESP32S2_R5 LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ESP32_Wilhelm_AR488_ESP32S2_R5

#define ESP32_ARDUINO_FUNC

#define DIO1_PIN   8    /* GPIB  8 */
#define DIO2_PIN   9    /* GPIB  9 */
#define DIO3_PIN   10   /* GPIB 10 */
#define DIO4_PIN   11   /* GPIB 11 */
#define DIO5_PIN   12   /* GPIB 12 */
#define DIO6_PIN   13   /* GPIB 13 */
#define DIO7_PIN   14   /* GPIB 14 */
#define DIO8_PIN   15   /* GPIB 15 */

#define IFC_PIN     4   /* GPIB  4 */
#define NDAC_PIN   33   /* GPIB 33 */
#define NRFD_PIN   34   /* GPIB 34 */
#define DAV_PIN     5   /* GPIB  5 */
#define EOI_PIN    35   /* GPIB 35 */

#define SRQ_PIN    37   /* GPIB 37 */
#define REN_PIN    21   /* GPIB 21 */
#define ATN_PIN    36   /* GPIB 36 */

#endif // ESP32_Wilhelm_AR488_ESP32S2_R5
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** Wilhelm_AR488_ESP32S2_R4 LAYOUT DEFINITION *****/
/******************************************************/


/*****************************************/
/***** STANDARD FUNCTION DEFINITIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
void initEspGpioPins();
void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** STANDARD FUNCTION DEFINITIONS *****/
/*****************************************/



#endif  // ESP32_H
