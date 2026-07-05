#ifndef MCP23S1X_H
#define MCP23S1X_H

#include "Arduino.h"
#include "../../../AR488_Config.h"


/***********************************************/
/***** MCP23S1X IC (SPI) LAYOUT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef MCP23S1X_EXPANDER

#include <SPI.h>

/***** NOTE: MCP23S1X pinout *****/

#define IFC_PIN    0  /* GPIB 9  : PORTA bit 0 */
#define NDAC_PIN   1  /* GPIB 8  : PORTA bit 1 */
#define NRFD_PIN   2  /* GPIB 7  : PORTA bit 2 */
#define DAV_PIN    3  /* GPIB 6  : PORTA bit 3 */
#define EOI_PIN    4  /* GPIB 5  : PORTA bit 4 */
#define REN_PIN    5  /* GPIB 17 : PORTA bit 5 */
#define SRQ_PIN    6  /* GPIB 10 : PORTA bit 6 */
#define ATN_PIN    7  /* GPIB 11 : PORTA bit 7 */

#define DIO1_PIN   0  /* GPIB 1  : PORTB bit 0 */
#define DIO2_PIN   1  /* GPIB 2  : PORTB bit 1 */
#define DIO3_PIN   2  /* GPIB 3  : PORTB bit 2 */
#define DIO4_PIN   3  /* GPIB 4  : PORTB bit 3 */
#define DIO5_PIN   4  /* GPIB 13 : PORTB bit 4 */
#define DIO6_PIN   5  /* GPIB 14 : PORTB bit 5 */
#define DIO7_PIN   6  /* GPIB 15 : PORTB bit 6 */
#define DIO8_PIN   7  /* GPIB 16 : PORTD bit 7 */

/***** NOTE: DIO1-DIO8 pins are assigned to ping 0 - 7 on port B *****/


/***** MCP23S1X defines *****/
// Direction registers
#define MCPDIRA  0x00
#define MCPDIRB  0x01

// Configuration register
#define MCPCON (0x0A)

// Pullup state register
#define MCPPUA 0x0C
#define MCPPUB 0x0D

// Port Register
#define MCPPORTA 0x12
#define MCPPORTB 0x13

// Interrupt registers
#define MCPINTENA 0x04    // Enable pin for interrupt on change (GPINTEN)
#define MCPINTCONA 0x08   // Configure interrupt: 0 = compare against previous; 1 = compare against DEFVAL
#define MCPINTFA 0x0E     // Flag that interrupt ocurred on pin (read-only)
#define MCPINTCAPA 0x10   // Read the status of the pin (read-only)
#define MCPINTPINA 2      // Pin assigned to catch MCP23S17 INTA on the Arduino controller

// MCP opcodes
#define MCPWRITE 0b01000000
#define MCPREAD  0b01000001


void readyGpibDbus(uint8_t state);
uint8_t readGpibDbus();
void setGpibDbus(uint8_t db);
void setGpibCtrlState(uint8_t bits, uint8_t mask);
void setGpibCtrlDir(uint8_t bits, uint8_t mask);
uint8_t getGpibPinState(uint8_t pin);

void mcpInit();
void mcpIntHandler();
uint8_t getMcpIntAReg();
uint8_t mcpByteRead(uint8_t reg);
void mcpByteWrite(uint8_t reg, uint8_t db);
uint8_t mcpDigitalRead(uint8_t pin);
void mcpInterruptsEn();


#endif // MCP23S1X_EXPANDER
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MCP23S1X IC (SPI) IC LAYOUT DEFINITION *****/
/**************************************************/

#endif  //  MCP23S1X.h
