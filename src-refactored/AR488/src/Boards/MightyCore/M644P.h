#ifndef M644P_H
#define M644P_H

#include "Arduino.h"
#include "../../../AR488_Config.h"



/********************************************************/
/***** PANDUINO/MIGHTYCORE MCGRAW LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef AVR_MEGA644P_MCGRAW

#define DIO1_PIN  10   /* GPIB 1  */
#define DIO2_PIN  11   /* GPIB 2  */
#define DIO3_PIN  12   /* GPIB 3  */
#define DIO4_PIN  13   /* GPIB 4  */
#define DIO5_PIN  14   /* GPIB 13 */
#define DIO6_PIN  15   /* GPIB 14 */
#define DIO7_PIN  16   /* GPIB 15 */
#define DIO8_PIN  17   /* GPIB 16 */

#define IFC_PIN   22   /* GPIB 9  */
#define NDAC_PIN  21   /* GPIB 8  */
#define NRFD_PIN  20   /* GPIB 7  */
#define DAV_PIN   19   /* GPIB 6  */
#define EOI_PIN   18   /* GPIB 5  */

#define SRQ_PIN   23   /* GPIB 10 */
#define REN_PIN   24   /* GPIB 17 */
#define ATN_PIN   31   /* GPIB 11 */

#endif // AVR_MEGA644P_MCGRAW
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** PANDUINO/MIGHTYCORE MCGRAW LAYOUT DEFINITION *****/
/********************************************************/


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);


#endif  // M644P_H
