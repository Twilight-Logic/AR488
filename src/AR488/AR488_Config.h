#ifndef AR488_CONFIG_H
#define AR488_CONFIG_H

/*********************************************/
/***** AR488 GLOBAL CONFIGURATION HEADER *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/


/***** Firmware version *****/
#define FWVER "AR488 GPIB controller, ver. 0.51.18, 26/02/2023"



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
//#define AR488_CUSTOM

/*
 * Configure the appropriate board/layout section
 * below as required
 */
#ifdef AR488_CUSTOM
  /* Board layout */
  /*
   * Define board layout in the AR488 CUSTOM LAYOUT
   * section below
   */
  /* Default serial port type */
  #define AR_SERIAL_TYPE_HW

/*** UNO and NANO boards ***/
#elif __AVR_ATmega328P__
  /* Board/layout selection */
  #define AR488_UNO
  #define AR488_NANO
  //#define AR488_MCP23S17
  //#define AR488_MCP23017

/*** MEGA 32U4 based boards (Micro, Leonardo) ***/
#elif __AVR_ATmega32U4__
  /*** Board/layout selection ***/
  #define AR488_MEGA32U4_MICRO  // Artag's design for Micro board
  //#define AR488_MEGA32U4_LR3  // Leonardo R3 (same pin layout as Uno)
  
/*** MEGA 2560 board ***/
#elif __AVR_ATmega2560__
  /*** Board/layout selection ***/
  #define AR488_MEGA2560_D
  //#define AR488_MEGA2560_E1
  //#define AR488_MEGA2560_E2
//  #define AR488_MCP23S17
  //#define AR488_MCP23017

/***** Panduino Mega 644 or Mega 1284 board *****/
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  /* Board/layout selection */
  #define AR488_MEGA644P_MCGRAW
  
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
//#define DEBUG_ENABLE
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
 * for SN7516x, MCP23S17 and MCP23017 chips.
 */


/***** Enable MCP23S17 chip *****/
/*
 * This version uses the SPI interface with speeds up to 10MHz max
 * Note: Use #define MCP23S17 as the layout definition
 */
#ifdef AR488_MCP23S17
  #define MCP_ADDRESS   0
  #define MCP_SELECTPIN 10
  #define MCP_INTERRUPT 2
#endif


/***** Enable MCP23017 chip *****/
/*
 * This version uses the I2C interface with speeds of 100kHz, 400kHz and 1,7MHz
 * Pull up resistors (4.7k) are required on the SDA and SCL pins
 * Note: Use #define MCP23017 as the layout definition
 */
#ifdef AR488_MCP23017
  #define MCP_ADDRESS   1
  #define MCP_INTERRUPT 3
#endif


/***** Enable SN7516x chips *****/
/*
 * Uncomment to enable the use of SN7516x GPIB tranceiver ICs.
 * This will require the use of an additional GPIO pin to control
 * the read and write modes of the ICs.
 */
//#define SN7516X
#ifdef SN7516X
//  #define SN7516X_TE 6
//  #define SN7516X_DC 13
//  #define SN7516X_SC 12
  // ONLYA board
  #define SN7516X_TE 13
  #define SN7516X_DC 5
#endif




/***** MISCELLANEOUS OPTIONS *****/
/*
 * Miscellaneous options
 */


/***** Pin State Detection *****/
/*
 * With UNO. NANO and MEGA boards with pre-defined layouts,
 * USE_INTERRUPTS can and should be used.
 * With the AR488_CUSTOM layout and unknown boards, USE_INTERRUPTS must  
 * be commented out. Interrupts are used on pre-defined AVR board layouts 
 * and will respond faster, however in-loop checking for state of pin states
 * can be supported with any board layout.
 */
/* 
#ifdef __AVR__
  // For supported boards use interrupt handlers
  #if defined (AR488_UNO) || defined (AR488_NANO) || defined (AR488_MEGA2560) || defined (AR488_MEGA32U4)
    #ifndef AR488_CUSTOM
      #ifndef AR488_MCP23S17
        #define USE_INTERRUPTS
      #endif
    #endif
  #endif
#endif
*/


/***** Local/remote signal (LED) *****/
//#define REMOTE_SIGNAL_PIN 7


/***** 8-way address DIP switch *****/
#define DIP_SWITCH
#ifdef DIP_SWITCH
#define DIP_SW_1  A0
#define DIP_SW_2  A1
#define DIP_SW_3  A2
#define DIP_SW_4  A3
#define DIP_SW_5  A4
#define DIP_SW_6  A5
#define DIP_SW_7  A6
#define DIP_SW_8  A7

#endif


/***** Acknowledge interface is ready *****/
//#define SAY_HELLO




/***** DEBUG LEVEL OPTIONS *****/
/*
 * Configure debug level options
 */

#ifdef DEBUG_ENABLE
  // Main module
  //#define DEBUG_SERIAL_INPUT    // serialIn_h(), parseInput_h()
  //#define DEBUG_CMD_PARSER      // getCmd()
  //#define DEBUG_SEND_TO_INSTR   // sendToInstrument();
  //#define DEBUG_SPOLL           // spoll_h()
  //#define DEBUG_DEVICE_ATN      // attnRequired()
  //#define DEBUG_IDFUNC          // ID command

  // AR488_GPIBbus module
  //#define DEBUG_GPIBbus_RECEIVE // GPIBbus::receiveData(), GPIBbus::readByte()
  //#define DEBUG_GPIBbus_SEND    // GPIBbus::sendData()
  //#define DEBUG_GPIBbus_CONTROL // GPIBbus::setControls() 
  //#define DEBUG_GPIB_COMMANDS   // GPIBbus::sendCDC(), GPIBbus::sendLLO(), GPIBbus::sendLOC(), GPIBbus::sendGTL(), GPIBbus::sendMSA() 
  //#define DEBUG_GPIB_ADDRESSING // GPIBbus::sendMA(), GPIBbus::sendMLA(), GPIBbus::sendUNT(), GPIBbus::sendUNL() 
  //#define DEBUG_GPIB_DEVICE     // GPIBbus::unAddressDevice(), GPIBbus::addressDevice
  
  // GPIB layout module
  //#define DEBUG_LAYOUTS

  // EEPROM module
  //#define DEBUG_EEPROM          // EEPROM

  // AR488 Bluetooth module
  //#define DEBUG_BLUETOOTH       // bluetooth
#endif


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** AR488 GLOBAL CONFIGURATION HEADER *****/
/*********************************************/


/*******************************/
/***** AR488 CUSTOM LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_CUSTOM

#define DIO1  A0  /* GPIB 1  */
#define DIO2  A1  /* GPIB 2  */
#define DIO3  A2  /* GPIB 3  */
#define DIO4  A3  /* GPIB 4  */
#define DIO5  A4  /* GPIB 13 */
#define DIO6  A5  /* GPIB 14 */
#define DIO7  4   /* GPIB 15 */
#define DIO8  5   /* GPIB 16 */

#define IFC   8   /* GPIB 9  */
#define NDAC  9   /* GPIB 8  */
#define NRFD  10  /* GPIB 7  */
#define DAV   11  /* GPIB 6  */
#define EOI   12  /* GPIB 5  */

#define SRQ   2   /* GPIB 10 */
#define REN   3   /* GPIB 17 */
#define ATN   7   /* GPIB 11 */

#endif
/***** ^^^^^^^^^^^^^^^^^^^ *****/
/***** AR488 CUSTOM LAYOUT *****/
/*******************************/



/********************************/
/***** AR488 MACROS SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvv *****/

/*
 * (See the AR488 user manual for details)
 */

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
 */
//#define USE_MACROS    // Enable the macro feature
//#define RUN_STARTUP   // Run MACRO_0 (the startup macro)

#ifdef USE_MACROS

/***** Startup Macro *****/

#define MACRO_0 "\
++addr 9\n\
++auto 2\n\
*RST\n\
:func 'volt:ac'\
"
/* End of MACRO_0 (Startup macro)*/

/***** User macros 1-9 *****/

#define MACRO_1 "\
++addr 3\n\
++auto 0\n\
M3\n\
"
/*<-End of macro*/

#define MACRO_2 "\
"
/*<-End of macro 2*/

#define MACRO_3 "\
"
/*<-End of macro 3*/

#define MACRO_4 "\
"
/*<-End of macro 4*/

#define MACRO_5 "\
"
/*<-End of macro 5*/

#define MACRO_6 "\
"
/*<-End of macro 6*/

#define MACRO_7 "\
"
/*<-End of macro 7*/

#define MACRO_8 "\
"
/*<-End of macro 8*/

#define MACRO_9 "\
"
/*<-End of macro 9*/


#endif
/***** ^^^^^^^^^^^^^^^^^^^^ *****/
/***** AR488 MACROS SECTION *****/
/********************************/


/******************************************/
/***** !!! DO NOT EDIT BELOW HERE !!! *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/



/*********************************************/
/***** MISCELLANEOUS DECLARATIONS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/

#define AR_CFG_SIZE 84

/******^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^******/
/***** MISCELLANEOUS DECLARATIONS *****/
/*********************************************/





#endif // AR488_CONFIG_H
