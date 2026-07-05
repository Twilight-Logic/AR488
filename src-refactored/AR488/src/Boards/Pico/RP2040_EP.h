#ifndef RP2040_EP_H
#define RP2040_EP_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/***************************************/
/***** RAS PICO LAYOUT 1 DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L1

void gpioFuncList();

#define DIO1_PIN   6   /* GPIB 1  */
#define DIO2_PIN   7   /* GPIB 2  */
#define DIO3_PIN   8   /* GPIB 3  */
#define DIO4_PIN   9   /* GPIB 4  */
#define DIO5_PIN  10   /* GPIB 13 */
#define DIO6_PIN  11   /* GPIB 14 */
#define DIO7_PIN  12   /* GPIB 15 */
#define DIO8_PIN  13   /* GPIB 16 */

#define IFC_PIN   14   /* GPIB 9  */
#define NDAC_PIN  15   /* GPIB 8  */
#define NRFD_PIN  16   /* GPIB 7  */
#define DAV_PIN   17   /* GPIB 6  */
#define EOI_PIN   18   /* GPIB 5  */

#define REN_PIN   19   /* GPIB 17 */
#define SRQ_PIN   20   /* GPIB 10 */
#define ATN_PIN   21   /* GPIB 11 */

#endif // RAS_PICO_L1
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO LAYOUT 1 DEFINITION *****/
/****************************************/



/****************************************/
/***** RAS PICO LAYOUT 2 DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L2

void gpioFuncList();

#define DIO1_PIN  14   /* GPIB 1  */
#define DIO2_PIN  15   /* GPIB 2  */
#define DIO3_PIN  16   /* GPIB 3  */
#define DIO4_PIN  17   /* GPIB 4  */
#define DIO5_PIN  18   /* GPIB 13 */
#define DIO6_PIN  19   /* GPIB 14 */
#define DIO7_PIN  20   /* GPIB 15 */
#define DIO8_PIN  21   /* GPIB 16 */

#define IFC_PIN    6   /* GPIB 9  */
#define NDAC_PIN   7   /* GPIB 8  */
#define NRFD_PIN   8   /* GPIB 7  */
#define DAV_PIN    9   /* GPIB 6  */
#define EOI_PIN   10   /* GPIB 5  */

#define REN_PIN   11   /* GPIB 17 */
#define SRQ_PIN   12   /* GPIB 10 */
#define ATN_PIN   13   /* GPIB 11 */

#endif // RAS_PICO_L2
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO LAYOUT 2 DEFINITION *****/
/****************************************/



/****************************************/
/***** RAS PICO LAYOUT 3 DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L3

void gpioFuncList();

#define DIO1_PIN   2   /* GPIB 1  */
#define DIO2_PIN   3   /* GPIB 2  */
#define DIO3_PIN   4   /* GPIB 3  */
#define DIO4_PIN   5   /* GPIB 4  */
#define DIO5_PIN   6   /* GPIB 13 */
#define DIO6_PIN   7   /* GPIB 14 */
#define DIO7_PIN   8   /* GPIB 15 */
#define DIO8_PIN   9   /* GPIB 16 */

#define IFC_PIN   10   /* GPIB 9  */
#define NDAC_PIN  11   /* GPIB 8  */
#define NRFD_PIN  12   /* GPIB 7  */
#define DAV_PIN   13   /* GPIB 6  */
#define EOI_PIN   14   /* GPIB 5  */

#define REN_PIN   15   /* GPIB 17 */
#define SRQ_PIN   20   /* GPIB 10 */
#define ATN_PIN   21   /* GPIB 11 */

#endif // RAS_PICO_L3
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO LAYOUT 3 DEFINITION *****/
/****************************************/


/****************************************/
/***** RAS PICO LAYOUT 4 DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L4

void gpioFuncList();

#define DIO1_PIN  10   /* GPIB 1  */
#define DIO2_PIN  11   /* GPIB 2  */
#define DIO3_PIN  12   /* GPIB 3  */
#define DIO4_PIN  13   /* GPIB 4  */
#define DIO5_PIN  14   /* GPIB 13 */
#define DIO6_PIN  15   /* GPIB 14 */
#define DIO7_PIN  20   /* GPIB 15 */
#define DIO8_PIN  21   /* GPIB 16 */

#define IFC_PIN    2   /* GPIB 9  */
#define NDAC_PIN   3   /* GPIB 8  */
#define NRFD_PIN   4   /* GPIB 7  */
#define DAV_PIN    5   /* GPIB 6  */
#define EOI_PIN    6   /* GPIB 5  */
#define REN_PIN    7   /* GPIB 17 */
#define SRQ_PIN    8   /* GPIB 10 */
#define ATN_PIN    9   /* GPIB 11 */

#endif // RAS_PICO_L4
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO LAYOUT 4 DEFINITION *****/
/****************************************/



/****************************************/
/***** RAS PICO LAYOUT 5 DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L5

#define RAS_PICO_LAUTO

void gpioFuncList();

#define DIO1_PIN  10   /* GPIB 1  */
#define DIO2_PIN  11   /* GPIB 2  */
#define DIO3_PIN  12   /* GPIB 3  */
#define DIO4_PIN  13   /* GPIB 4  */
#define DIO5_PIN  14   /* GPIB 13 */
#define DIO6_PIN  15   /* GPIB 14 */
#define DIO7_PIN  20   /* GPIB 15 */
#define DIO8_PIN  21   /* GPIB 16 */

#define IFC_PIN    9   /* GPIB 9  */
#define NDAC_PIN   8   /* GPIB 8  */
#define NRFD_PIN   7   /* GPIB 7  */
#define DAV_PIN    6   /* GPIB 6  */
#define EOI_PIN    5   /* GPIB 5  */
#define REN_PIN    3   /* GPIB 17 */
#define SRQ_PIN    4   /* GPIB 10 */
#define ATN_PIN    2   /* GPIB 11 */

#endif // RAS_PICO_L5
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO LAYOUT 5 DEFINITION *****/
/****************************************/



/***********************************************/
/***** ADAFRUIT RP2040 ADLOGGER DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ADAFRUIT_ADLOGGER

void gpioFuncList();

#define DIO1_PIN  12   /* GPIB 1  */
#define DIO2_PIN  11   /* GPIB 2  */
#define DIO3_PIN  10   /* GPIB 3  */
#define DIO4_PIN   9   /* GPIB 4  */
#define DIO5_PIN   6   /* GPIB 13 */
#define DIO6_PIN   5   /* GPIB 14 */
#define DIO7_PIN   3   /* GPIB 15 */
#define DIO8_PIN   2   /* GPIB 16 */

#define IFC_PIN    4   /* GPIB 9  */
#define NDAC_PIN   0   /* GPIB 8  */
#define NRFD_PIN   1   /* GPIB 7  */
#define DAV_PIN    8   /* GPIB 6  */
#define EOI_PIN   15   /* GPIB 5  */

#define REN_PIN   13   /* GPIB 17 */
#define SRQ_PIN   25   /* GPIB 10 */
#define ATN_PIN   14   /* GPIB 11 */

#endif // ADAFRUIT_ADLOGGER
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ADAFRUIT RP2040 ADLOGGER DEFINITION *****/
/***********************************************/

#if defined(RP2040_COMMON_FUNCTIONS)
  void gpioFuncList();
#endif

void gpio_set_pullups_masked(uint32_t mask);
void gpio_clear_pullups_masked(uint32_t mask);
void initRpGpioPins();
void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);


#endif  // RP2040_EP_H

