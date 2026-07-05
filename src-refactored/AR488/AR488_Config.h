#ifndef AR488_CONFIG_H
#define AR488_CONFIG_H


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||              AR488 CONFIGURATION HEADER FILE                ||
||                                                             ||
\*=============================================================*/

#define FW_VERSION "AR488 GPIB controller, ver. 0.55.22, 21/06/2026"



#define INTERFACE_PROLOGIX

// Define modules to include
// Note: USE_PROLOGIX_COMMON must always be enabled
#ifdef INTERFACE_PROLOGIX
  #define USE_PROLOGIX_COMMON 
  #define USE_PROLOGIX_CONTROLLER
  #define USE_PROLOGIX_DEVICE
  #define USE_RATTN_HANDLER
  #define USE_EEPROM_HANDLER
  #define USE_MACRO_HANDLER_PM
  #define USE_DIAGNOSTIC_HELPER
#endif


/***** BOARD CONFIGURATION *****/
/*
 * Platform will be selected automatically based on 
 * Arduino definition.
 * Only ONE board/layout should be selected per platform
 * Only ONE Serial port can be used to receive output
 */


/*** Custom layout ***/
/*
 * Uncomment to use custom board layout
 */
//#define AR488_CUSTOM_LAYOUT

/*
 * Configure the appropriate board/layout section
 * below as required
 */
#ifdef AR488_CUSTOM_LAYOUT
/* Board layout */
/*
   * Define board layout in /Boards/Custom/Custom.h
   */

#elif defined(__AVR_ATmega328P__)
  /*** ATmega328P - UNO R3, Nano ***/
  #define AVR_328P_UNO
  //#define AVR_328P_NANO
  //#define MCP23S1X_EXPANDER

#elif defined(__AVR_ATmega328PB__)
  /** ATmega 328PB variant - some clone Nano boards **/
  #define MINICORE_NANO
  //#define MINICORE_NANO_ALT

#elif defined(__AVR_ATmega32U4__)
  /** ATmega 32u4 - Micro, Leonardo  **/
  #define AVR_32U4_MICRO  // Artag's design for Micro board
  //#define AVR_32U4_LEO_R3  // Leonardo R3 (same pin layout as Uno)

#elif defined(__AVR_ATmega2560__)
  /** ATmega2560 - Mega 2560 **/
  #define AVR_MEGA2560_DF
  //#define AVR_MEGA2560_E1
  //#define AVR_MEGA2560_E2

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  /** ATmega 644P, ATmega 1284P, e.g. Panduino **/
  #define AVR_MEGA644P_MCGRAW

#elif defined(__AVR_ATmega4809__)
  /** ATmega4809 - Nano Every, UNO WiFi Rev2 **/
  #define KOFEN_POE_ETHERNET_ADAPTOR

#elif defined(ESP32)
  /** ESP32 variants **/
  #define ESP32_DEVKIT1_WROOM_32
  // David Douard / Johann Wilhelm board layouts
  //#define ESP32_TTGO_T8_161
  //#define ESP32_ESP32DEV
  //#define ESP32_LOLIN32_161 // Note: ESP32_LOLIN32_161_V2 profile has the same pin assigments
  //#define ESP32_S2_161
  //#define ESP32_Wilhelm_AR488_ESP32S2_R4
  //#define ESP32_Wilhelm_AR488_ESP32S2_R5

#elif defined(ARDUINO_ARCH_RP2040)
  /** RP2040 & RP2350 Boards **/
  //#define RAS_PICO_L1         // SPI on pins 4-7, data on lower pins
  //#define RAS_PICO_L2         // SPI on pins 4-7, control signals on lower pins
  //#define RAS_PICO_L3         // SPI on pins 16-19, data on lower pins
  //#define RAS_PICO_L4         // SPI on pins 16-19, control signals on lower pins
  #define RAS_PICO_L5

//#elif defined(ARDUINO_NANO_RP2040_CONNECT)

//#elif defined(ARDUINO_ARCH_MBED_NANO)

//#elif defined(ARDUINO_ARCH_MBED_RP2040)

#elif defined(ARDUINO_ARCH_RENESAS)
  /** UNO/NANO (Renesas) R4 boards **/
  /* NOTE: Renesas RA4M1 boards work only with SN7516x buffer chips */
  #define RA4M1_NANO_R4

#elif defined(__IMXRT1062__)
  /** Teensy 4.1. boards **/
  #define IMXRT1062_TEENSY41_01

#endif  // Board/layout selection





/***** SERIAL PORT CONFIGURATION *****/
/*
 * Note: On most boards the primary serial device is named Serial. On boards that have a secondary
 *       UART port this is named Serial1. The Mega2560 also supports Serial2, Serial3 and Serial4.
 *       When using layout AR488_MEGA2560_D, Serial2 pins are assigned as GPIO pins for the GPIB bus
 *       so Serial2 is not available.
 */
/***** Communication port *****/
#define DATAPORT_ENABLE
#ifdef DATAPORT_ENABLE
  // Serial port device
  #define AR_SERIAL_PORT Serial
  // #define AR_SERIAL_SWPORT
  // Set port operating speed
  #define AR_SERIAL_SPEED 115200
  // Enable Bluetooth (HC05) module?
  //#define AR_SERIAL_BT_ENABLE 12        // HC05 enable pin
  //#define AR_SERIAL_BT_NAME "AR488-BT"  // Bluetooth device name
  //#define AR_SERIAL_BT_CODE "488488"    // Bluetooth pairing code
#endif

/***** Debug port *****/
#define DEBUG_ENABLE
#ifdef DEBUG_ENABLE
  // Serial port device
  #define DB_SERIAL_PORT Serial
  // #define DB_SERIAL_SWPORT
  // Set port operating speed
  #define DB_SERIAL_SPEED 115200
#endif

/***** Configure SoftwareSerial Port *****/
/*
 * Configure the SoftwareSerial TX/RX pins and baud rate here
 * Note: SoftwareSerial support conflicts with PCINT support
 * When using SoftwareSerial, disable USE_INTERRUPTS.
 */
#if defined(AR_SERIAL_SWPORT) || defined(DB_SERIAL_SWPORT)
  #define SW_SERIAL_RX_PIN 11
  #define SW_SERIAL_TX_PIN 12
#endif
/*
 * Note: SoftwareSerial reliable only up to a MAX of 57600 baud only
 */



/***** SUPPORT FOR PERIPHERAL CHIPS *****/
/*
 * This sections priovides configuration to enable/disable support
 * for SN7516x chips and the MCP23S17 GPIO expander.
 */


/***** Enable MCP23S17 GPIO expander chip *****/
/*
 * This version uses the SPI interface with speeds up to 10MHz max
 * Note: Use #define MCP23S17 as the layout definition
 */
#ifdef MCP23S1X_EXPANDER
  #define MCP_ADDRESS 0
  #define MCP_SELECTPIN 10
  #define MCP_INTERRUPT 2
#endif


/***** Enable SN7516x chips *****/
/*
 * Uncomment to enable the use of SN7516x GPIB tranceiver ICs.
 * This will require the use of an additional GPIO pin to control
 * the read and write modes of the ICs.
 *
 * Signals:
 * #define SN7516X_TE - pin assigned to Talk Enable (TE)
 * #define SN7515X_DC - pin assigned to Direction Control (DC) - can be driven by REN
 * #define SN7516X_SC - pin assigned to System Control (SC) - can be driven by REN
 *
 * Note: SC is only present on the SN75162. 
 */
//#define SN7516X
#ifdef SN7516X
  /*** Jay Diddy B board ***/
  //  #define SN7516X_TE 6
  //  #define SN7516X_DC 13
  //  #define SN7516X_SC 12
  /*** ONLYA board ***/
  //  #define SN7516X_TE 13
  //  #define SN7516X_DC 5
  /*** WilheJo board (V4) ***/
  //  #define SN7516X_TE 17
  //  #define SN7516X_DC 45
  /*** Devkit v1 ***/
  //  #define SN7516X_TE 2
  // DC to REN
  /*** Pico RP2040 ***/
  //  #define SN7516X_TE 22
  // DC to REN
  /*** Teensy 4.1 ***/
  #define SN7516X_TE 2
  //  #define SN7516X_DC 3
#endif



/***** MISCELLANEOUS OPTIONS *****/
/*
 * Miscellaneous options
 */


/***** Device mode local/remote signal (LED) *****/
//#define REMOTE_SIGNAL_PIN 7


/***** Acknowledge interface is ready *****/
//#define SAY_HELLO



/***** Storage devices *****/
/*
 * Only ONE storage device can be used at a time!
 */
//#define ENABLE_STORAGE
/*
#ifdef ENABLE_STORAGE
  #define SDCARD_CS_PIN 4         // SS pin on Pandauino 644-Narrow / 1284-Narrow (644P/1284P)
  #define SDCARD_CLK 16           // Clock speed in MHz
//  #define STORAGE_USES_ADDR_0X60  // If un-commented, address 0x60 will be handled by storage
  #define ENABLE_TEK_4924         // Emulate Tektronix 4924 Tape drive
//  #define ENABLE_PET_9060
#endif
*/

/***** Device mode LISTEN handler *****/
//#define FAKE_LISTEN_HANDLER
//#define DEVICE_IDN_TEXT "Tektronix Flash Drive v0.6.15\r"


/***** DEBUG LEVEL OPTIONS *****/
/*
 * Configure debug level options
 */

#ifdef DEBUG_ENABLE

  // Enable millis timestamp
  // #define DEBUG_MILLIS_TIMESTAMP

  // Main module
//  #define DEBUG_PARSER            // parser.serialIn_h(), parser.parseInput_h(), parser.parseMacro()
//  #define DEBUG_PCOM_CMD_HANDLER    // Prologix_Common module: execCmd(), runCmd()
//  #define DEBUG_PCTL_CMD_HANDLER    // Prologix Controller module: execCmd(), runCmd()
//  #define DEBUG_PDEV_CMD_HANDLER    // Prologix Device module: execCmd(), runCmd()
//  #define DEBUG_EEPM_CMD_HANDLER    // EEPROM Handler module: execCmd(), runCmd()
//  #define DEBUG_MAPM_CMD_HANDLER    // Macro Handler PM module: execCmd(), runCmd()
//  #define DEBUG_RATN_CMD_HANDLER    // RATTN Handler module: execCmd(), runCmd()
//  #define DEBUG_DIAG_CMD_HANDLER    // Diagnostic module: execCmd(), runCmd()

  //#define DEBUG_CMD_HANDLER       // execCmd(), runCmd()
  //#define DEBUG_SEND_TO_INSTR     // sendToInstrument();
  //#define DEBUG_SPOLL             // spoll_h()
  //#define DEBUG_IDFUNC            // ID command

  // AR488_GPIBbus module
  //#define DEBUG_GPIBbus_RECEIVE   // GPIBbus::receiveData(), GPIBbus::readByte()
  //#define DEBUG_GPIBbus_SEND      // GPIBbus::sendData()
  //#define DEBUG_GPIBbus_CONTROL   // GPIBbus::setControls()
  //#define DEBUG_GPIB_COMMANDS     // GPIBbus::sendCDC(), GPIBbus::sendLLO(), GPIBbus::sendLOC(), GPIBbus::sendGTL(), GPIBbus::sendMSA()
  //#define DEBUG_GPIB_ADDRESSING   // GPIBbus::sendUNT(), GPIBbus::sendUNL()
  //#define DEBUG_GPIB_DEVICE       // GPIBbus::unAddressDevice(), GPIBbus::addressDevice

  // GPIB layout
  //#define DEBUG_LAYOUT

  // EEPROM module
  //#define DEBUG_EEPROM_MODULE

  // RATTN module
  //#define DEBUG_RATTN_MODULE

  // Macro module
//  #define DEBUG_MACRO_PM_MODULE

  // AR488 Bluetooth module
  //#define DEBUG_BLUETOOTH       // bluetooth
#endif


/***** EEPROM SUPPORT *****/
/* Note: maybe need to add E2END ? */

#if defined(USE_EEPROM_HANDLER)
#if defined(__AVR__) || defined(ESP32) || defined(ESP8266) || defined(ARDUINO_ARCH_RENESAS) || defined(__IMXRT1062__)
#define EEPROM_ENABLED
#endif
#endif


/***** Enable Macros *****/
/*
 * Uncomment to enable macro support. The Startup macro allows the
 * interface to be configured at startup. Macros 1 - 9 can be
 * used to execute a sequence of commands with a single command
 * i.e, ++macro n, where n is the number of the macro
 * 
 * USE_MACROS must be enabled to enable the macro feature including 
 * MACRO_0 (the startup macro). RUN_STARTUP must be uncommented to 
 * run the startup macro when the interface boots up
 *
 * (See the AR488 user manual for details)
 *
 */

//#define USE_MACRO_HELPER    // Enable the macro feature
//#define RUN_STARTUP_MACRO   // Run MACRO_0 (the startup macro)


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** AR488 GLOBAL CONFIGURATION HEADER *****/
/*********************************************/





/*==================================================*/
/********** !!! DO NOT EDIT BELOW HERE !!! **********/
/*==================================================*/


/*********************************************/
/***** MISCELLANEOUS DECLARATIONS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/

#define AR_CFG_SIZE 84

#define FUNC_TYPE_VOID  0x40
#define FUNC_TYPE_PARAM 0x80

/******^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^******/
/***** MISCELLANEOUS DECLARATIONS *****/
/*********************************************/



/**********************************/
/***** BOARD LAYOUT LIBRARIES *****/
/******vvvvvvvvvvvvvvvvvvvvvv******/

#include "src/Boards/AVR/M328p.h"
#include "src/Boards/AVR/M32u4.h"
#include "src/Boards/AVR/M2560.h"
#include "src/Boards/MiniCore/M328PB.h"
#include "src/Boards/MightyCore/M644P.h"
#include "src/Boards/MegaCoreX/M4809.h"
#include "src/Boards/Pico/RP2040_EP.h"
#include "src/Boards/Espressif/ESP32.h"
#include "src/Boards/Renesas/RA4M1.h"
#include "src/Boards/Teensy/Teensy41.h"

#include "src/Boards/Custom/Custom.h"

#include "src/Boards/Expander/MCP23S1X.h"

/******^^^^^^^^^^^^^^^^^^^^^^******/
/***** BOARD LAYOUT LIBRARIES *****/
/**********************************/


#endif  // AR488_CONFIG_H
