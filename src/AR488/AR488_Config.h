#ifndef AR488_CONFIG_H
#define AR488_CONFIG_H

/*********************************************/
/***** AR488 GLOBAL CONFIGURATION HEADER *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/


/***** Firmware version *****/
#define FWVER "AR488 GPIB controller, ver. 0.49.12, 11/01/2021"


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
  /* Serial ports */
  #define AR_HW_SERIAL
  #define AR_SERIAL_PORT Serial
  //#define AR_SERIAL_PORT Serial1
  //#define AR_SERIAL_PORT Serial2
  //#define AR_SERIAL_PORT Serial3
  //#define AR_CDC_SERIAL
  //#define AR_SW_SERIAL

/*** UNO and NANO boards ***/
#elif __AVR_ATmega328P__
  /* Board/layout selection */
  #define AR488_UNO
  //#define AR488_NANO
  /*** Serial ports ***/
  //Select HardwareSerial or SoftwareSerial (default = HardwareSerial) ***/
  // The UNO/NANO default hardware port is 'Serial'
  // (Comment out #define AR_HW_SERIAL if using SoftwareSerial)
  #define AR_HW_SERIAL
  #ifdef AR_HW_SERIAL
    #define AR_SERIAL_PORT Serial
  #else
    // Select software serial port
    #define AR_SW_SERIAL
  #endif

/*** MEGA 32U4 based boards (Micro, Leonardo) ***/
#elif __AVR_ATmega32U4__
  /*** Board/layout selection ***/
  #define AR488_MEGA32U4_MICRO  // Artag's design for Micro board
  //#define AR488_MEGA32U4_LR3  // Leonardo R3 (same pin layout as Uno)
  /*** Serial ports ***/
  // By default the CDC serial port is used
  // Comment out #define AR_CDC_SERIAL if using RXI, TXO pins
  #define AR_CDC_SERIAL
  #ifdef AR_CDC_SERIAL
    // The Mega 32u4 default port is a virtual USB CDC port named 'Serial'
    #define AR_SERIAL_PORT Serial
  #else
    // Use hardware port Serial1
    #define AR_HW_SERIAL
    #define AR_SERIAL_PORT Serial1
  #endif
  
/*** MEGA 2560 board ***/
#elif __AVR_ATmega2560__
  /*** Board/layout selection ***/
  #define AR488_MEGA2560_D
  //#define AR488_MEGA2560_E1
  //#define AR488_MEGA2560_E2
  /*** Serial ports ***/
  // Mega 2560 supports Serial, Serial1, Serial2, Serial3. Since the pins 
  // associated with Serial2 are used in the default pin layout, Serial2
  // is unavailable. The default port is 'Serial'. Choose ONE port only.
/*  
  // and associated SERIALEVENT definition
*/
  #define AR_HW_SERIAL
  #define AR_SERIAL_PORT Serial
  //#define AR_SERIAL_PORT Serial1
  //#define AR_SERIAL_PORT Serial3

#endif  // Board/layout selection


/***** Software Serial Support *****/
/*
 * Configure the SoftwareSerial TX/RX pins and baud rate here
 * Note: SoftwareSerial support conflicts with PCINT support
 * When using SoftwareSerial, disable USE_PCINTS and enable 
 * USE_PINHOOKS
 */
#ifdef AR_SW_SERIAL
  #define AR_SW_SERIAL_RX 53
  #define AR_SW_SERIAL_TX 51
  #define AR_SW_SERIAL_BAUD 57600
#else
  #define AR_SERIAL_BAUD 115200
#endif
/*
 * Note: SoftwareSerial reliable only up to a MAX of 57600 baud only
 */


/***** Pin State Detection *****/
/*
 * With UNO. NANO and MEGA boards with pre-defined layouts,
 * USE_PCINTS can be used.
 * With the AR488_CUSTOM layout and unknown boards, USE_PINHOOKS must  
 * be used. Interrupts are used on pre-defined AVR board layouts and will 
 * respond faster, however "pinhooks" (in-loop checking for state of pin) 
 * can be supported with any board layout.
 */
#ifdef __AVR__
  // For supported boards use interrupt handlers
  #if defined (AR488_UNO) || defined (AR488_NANO) || defined (AR488_MEGA2560) || defined (AR488_MEGA32U4)
    #ifndef AR488_CUSTOM
      #define USE_INTERRUPTS
    #endif
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
 */
//#define USE_MACROS    // Enable the macro feature
//#define RUN_STARTUP   // Run MACRO_0 (the startup macro)


/***** Enable SN7516x chips *****/
/*
 * Uncomment to enable the use of SN7516x GPIB tranceiver ICs.
 * This will require the use of an additional GPIO pin to control
 * the read and write modes of the ICs.
 */
//#define SN7516X
#ifdef SN7516X
  #define SN7516X_TE 6
//  #define SN7516X_DC 13
//  #define SN7516X_SC 12
#endif


/***** Bluetooth (HC05) support *****/
/*
 * Uses built-in LED on GPIO pin 13 to signal status
 */
//#define AR_BT_EN 12             // Bluetooth enable and control pin
#ifdef AR_BT_EN
  #define AR_BT_BAUD 115200     // Bluetooth module preferred baud rate
  #define AR_BT_NAME "AR488-BT" // Bluetooth device name
  #define AR_BT_CODE "488488"   // Bluetooth pairing code
#endif


/***** Acknowledge interface is ready *****/
//#define SAY_HELLO

/***** Debug options *****/
// Uncomment to send debug messages to another port
//#define DB_SERIAL_PORT Serial1
// Configure alternative port for debug messages
#define DB_SERIAL_BAUD 115200
#define DB_HW_SERIAL
#ifdef DB_SW_SERIAL
  #define DB_SW_SERIAL_RX 53
  #define DB_SW_SERIAL_TX 51
#endif
// Configure debug level
//#define DEBUG1  // getCmd
//#define DEBUG2  // setGpibControls
//#define DEBUG3  // gpibSendData
//#define DEBUG4  // spoll_h
//#define DEBUG5  // attnRequired
//#define DEBUG6  // EEPROM
//#define DEBUG7  // gpibReceiveData
//#define DEBUG8  // ppoll_h
//#define DEBUG9  // bluetooth
//#define DEBUG10 // ID command


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
/***** SERIAL PORT EXTERNAL DECLARATIONS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/

#ifdef AR_CDC_SERIAL
  extern Serial_ *arSerial;
  #ifndef DB_SERIAL_PORT
    extern Serial_ *dbSerial;
  #endif
#endif
#ifdef AR_HW_SERIAL
  extern HardwareSerial *arSerial;
  #ifndef DB_SERIAL_PORT
    extern HardwareSerial *dbSerial;
  #endif
#endif
// Note: SoftwareSerial support conflicts with PCINT support
#ifdef AR_SW_SERIAL
  #include <SoftwareSerial.h>
  extern SoftwareSerial *arSerial;
  #ifndef DB_SERIAL_PORT
    extern SoftwareSerial *dbSerial;
  #endif
#endif


/***** Debug Port *****/
#ifdef DB_SERIAL_PORT
  #ifdef DB_CDC_SERIAL
    extern Serial_ *dbSerial;
  #endif
  #ifdef DB_HW_SERIAL
    extern HardwareSerial *dbSerial;
  #endif
  // Note: SoftwareSerial support conflicts with PCINT support
  #ifdef DB_SW_SERIAL
    #include <SoftwareSerial.h>
    extern SoftwareSerial *dbSerial;
  #endif
#endif

/******^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^******/
/***** SERIAL PORT EXTERNAL DECLARATIONS *****/
/*********************************************/


/*********************************************/
/***** MISCELLANEOUS DECLARATIONS *****/
/******vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv******/

#define AR_CFG_SIZE 84

/******^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^******/
/***** MISCELLANEOUS DECLARATIONS *****/
/*********************************************/





#endif // AR488_CONFIG_H
