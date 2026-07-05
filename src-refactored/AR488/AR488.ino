//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wtype-limits"
//#pragma GCC diagnostic ignored "-Wunused-variable"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.22, 05/07/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                      MAIN SKETCH FILE                       ||
||                                                             ||
\*=============================================================*/


/*
  Arduino IEEE-488 implementation by John Chajecki

  Inspired by the original work of Emanuele Girlando, licensed under a Creative
  Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
  Any code in common with the original work is reproduced here with the explicit
  permission of Emanuele Girlando, who has kindly reviewed and tested the
  initial version of this project code.

  Thanks also to Luke Mester for comparison testing against the Prologix interface.
  AR488 is Licenced under the GNU Public licence.

  Thanks to 'maxwell3e10' on the EEVblog forum for suggesting additional auto mode
  settings and the macro feature.

  Thanks to 'artag' on the EEVblog forum for providing code for the 32u4.
*/

/*
  Implements most of the CONTROLLER functions;
  Substantially compatible with 'standard' Prologix "++" commands
  (see +savecfg command in the manual for differences)

  Principle of operation:
  - Commands received from USB are buffered and whole terminated lines processed
  - Interface commands prefixed with "++" are passed to the command handler
  - Instrument commands and data not prefixed with '++' are sent directly to the GPIB bus.
  - the *IDN? command can be optionally intercepted to allow the AR488 to send an ID string
  - To receive from the instrument, issue a ++read command or put the controller in auto mode (++auto 1|2|3)
  - Characters received over the GPIB bus are unbuffered and sent directly to USB
   
  NOTES:
  - GPIB line in a HIGH state is un-asserted (inactive)
  - GPIB line in a LOW state is asserted (active)
  - When powered on, ATMega MCU GPIO pins have a high impedance when set as inputs
  - When set to INPUT_PULLUP, a 10k pull-up (to VCC) resistor is applied to the input
  - when powered off, ATMega MCU GPIO pins are NOT high-Z

  NOT YET IMPLEMENTED
  ++myaddr   - set the controller address

*/

/*
  For information regarding the GPIB firmware by Emanualle Girlando see:
  http://egirland.blogspot.com/2014/03/arduino-uno-as-usb-to-gpib-controller.html
*/

/*
  NOTES:
  Details of pin mapping between the Arduino and the GPIB connector can be found in
  the .h file for each given board/MCU type combo that can be found in the ./Boards
  directory.

  A custom pim mapping can be created by editing the /Boards/Custom/Custom.h file.
  The created layout can be enabled by un-commnting the #AR488_CUSTOM_LAYOUT line
  in the AR488_Config.h file.

  GPIB connector pins 10 and 18-24 are connected to GND
  GPIB connector pin 12 should be connected to the cable shield (might be n/c)

  The UNO/NANO pin mapping follows the layout originally used by Emanuelle Girlando,
  but adds the SRQ line (GPIB 10) on pin 2 and the REN line (GPIB 17) on pin 13. The
  program should therefore be compatible with the original interface design but for
  full functionality will need the remaining two pins to be connected.

  For further information about the AR488 see the AR488 Manual. 
*/

//#pragma GCC diagnostic pop


#ifdef __AVR__
  #include <avr/wdt.h>
#endif

#include "AR488_Config.h"
#include "AR488_ComPorts.h"
#include "AR488_Modules.h"
#include "GPIBbus_Handler.h"


AR488modulesList AR488modules;
GPIBbus gpibBus;


#ifdef INTERFACE_PROLOGIX

  #include "Utility_Helper.h"
  #include "Prologix_Parser.h"

  inputBuffer inBuffer;
  utilityHelper utils;

  #ifdef USE_PROLOGIX_COMMON
    #include "Prologix_Common.h"
    prologixCommon pCommon(gpibBus, utils);
  #endif

  #ifdef USE_PROLOGIX_CONTROLLER
    #include "Prologix_Controller.h"
    prologixController pController(gpibBus, utils);
  #endif

  #ifdef USE_PROLOGIX_DEVICE
    #include "Prologix_Device.h"
    prologixDevice pDevice(gpibBus, utils);
  #endif

  #ifdef USE_EEPROM_HANDLER
    #include "EEPROM_Handler.h"
    eepromHandler epHandler(gpibBus, utils);
  #endif

  #ifdef USE_MACRO_HANDLER_PM
    #include "Macro_Handler_PM.h"
    macroHandlerPM mHandlerPm(inBuffer, utils);
  #endif

  #ifdef USE_DIAGNOSTIC_HELPER
    #include "Diagnostic_Helper.h"
    Diagnostix diags(gpibBus, utils);
  #endif

  #ifdef USE_RATTN_HANDLER
    #include "RATTN_Handler.h"
    rattnHandler atnHandler(gpibBus, inBuffer, utils);
  #endif

  prologixParser prolParser(inBuffer, gpibBus, utils, AR488modules);


#endif  // INTERFACE_PROLOGIX



/************************************/
/***** COMMON VARIABLES SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/


bool isVerb;


/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** COMMON VARIABLES SECTION *****/
/************************************/



/*******************************/
/***** COMMON CODE SECTION *****/
/***** vvvvvvvvvvvvvvvvvvv *****/

void initModules(){
  #ifdef USE_PROLOGIX_COMMON
    AR488modules.pcommon = &pCommon;
  #endif

  #ifdef USE_PROLOGIX_CONTROLLER
    AR488modules.pcontroller = &pController;
  #endif

  #ifdef USE_PROLOGIX_DEVICE
    AR488modules.pdevice = &pDevice;
  #endif

  #ifdef USE_RATTN_HANDLER
    AR488modules.rattn = &atnHandler;
  #endif

  #ifdef USE_EEPROM_HANDLER
    AR488modules.eeprom = &epHandler;
  #endif

  #ifdef USE_MACRO_HANDLER_PM
    AR488modules.macropm = &mHandlerPm;
  #endif

  #ifdef USE_DIAGNOSTIC_HELPER
    AR488modules.diag = &diags;
  #endif
}


/****** Arduino SETUP procedure *****/
void setup() {

  // Disable the watchdog (needed to prevent WDT reset loop)
  #ifdef __AVR__
    wdt_disable();
  #endif

  // Turn off internal LED (set OUPTUT/LOW) - Arduinos have a separate power LED
  #ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
  #endif

  // For RPI we turn on the built-in LED as a power indicator
  #ifdef ARDUINO_ARCH_RP2040
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
  #endif

  #ifdef REMOTE_SIGNAL_PIN
    pinMode(REMOTE_SIGNAL_PIN, OUTPUT);
    digitalWrite(REMOTE_SIGNAL_PIN, LOW);
  #endif

  // Initialise serial at the configured baud rate
  AR_SERIAL_PORT.begin(AR_SERIAL_SPEED);
 
  #ifdef DEBUG_ENABLE
    // Initialise debug port
    DB_SERIAL_PORT.begin(DB_SERIAL_SPEED);
  #endif

  #ifdef AR_SERIAL_BT_ENABLE
    // If enabled, initialise Bluetooth
    /* If its the same interface as AR_SERIAL_PORT then there will be
    * some disruption while the function auto-detects the HC05 baud
    * rate and configures the HC05. Once this is done, AR_SERIAL_PORT
    * will be set back to the configured baud rate.
    */
    btInit();
  #endif

  // Initialise AR488 Modules
  initModules();

  #ifdef USE_EEPROM_HANDLER
    epHandler.loadCfg();
  #endif


  // Using MCP23S17 (SPI) expander chip
  #ifdef AR488_MCP23S17
    // Ensure the Arduino MCP select pin is set as an OUPTPUT and is HIGH
    pinMode(MCP_SELECTPIN, OUTPUT);
    digitalWrite(MCP_SELECTPIN, HIGH);
    // Enable SPI and initialise the MCP chip
    //Serial.println(F("Starting SPI..."));
    mcpInit();
    // Attach interrupt handler to Arduino board pin to receive MCP23S17 interrupt
    attachInterrupt(digitalPinToInterrupt(MCP_INTERRUPT), mcpIntHandler, FALLING);
    //Serial.println(F("SPI started."));
  #endif


// Un-comment for diagnostic purposes
  /* 
  #if defined(__AVR_ATmega32U4__)
    while(!AR_SERIAL_PORT)
    ;
  for(int i = 0; i < 20; ++i) {  // this gives you 10 seconds to start programming before it crashes
    dataport.print(".");
    delay(500);
  }
  dataport.println("@>");
  #endif // __AVR_ATmega32U4__
  */
  // while(!AR_SERIAL_PORT);  // Wait for serial port
// Un-comment for diagnostic purposes


// SN7516x IC support
/*  
#ifdef SN7516X
  pinMode(SN7516X_TE, OUTPUT);
  #ifdef SN7516X_DC
    pinMode(SN7516X_DC, OUTPUT);
  #endif
  if (gpibBus.cfg.cmode==2) {
    // Set controller mode on SN75161/2
    digitalWrite(SN7516X_TE, LOW);
    #ifdef SN7516X_DC 
      digitalWrite(SN7516X_DC, LOW);
    #endif
    #ifdef SN7516X_SC
      digitalWrite(SN7516X_SC, HIGH);
    #endif
  }else{
    // Set listen mode on SN75161/2 (default)
    digitalWrite(SN7516X_TE, HIGH);
    #ifdef SN7516X_DC
      digitalWrite(SN7516X_DC, HIGH);
    #endif
    #ifdef SN7516X_SC
      digitalWrite(SN7516X_SC, LOW);
  bool eoiDetected = false;
    #endif
  }
#endif
*/

  #ifdef INTERFACE_PROLOGIX
    // Initialise configured GPIB mode
    #ifdef USE_PROLOGIX_CONTROLLER
      if (gpibBus.cfg.cmode == 2) pController.init();
    #endif
    #ifdef USE_PROLOGIX_DEVICE
      if (gpibBus.cfg.cmode == 1) pDevice.init();
    #endif

    // Initialise the prologix parser
    prolParser.init();

    // Initialise the ATN handler
    #ifdef USE_RATTN_HANDLER
    #endif
  #endif

  // Clear the data port
  dataPort.flush();

}
/****** End of SETUP procedure *****/


/***** Arduino main loop *****/
void loop() {

//  Serial1.println(F("Hello :-)"));

#ifdef INTERFACE_PROLOGIX
  if (dataPort.available()) prolParser.parse();

  #ifdef USE_PROLOGIX_DEVICE

    if (pDevice.isLonEnabled()) {
      pDevice.lonMode(dataPort);
    }else if (pDevice.isTonEnabled()) {
/*
      if (prolParser.ready()==2) {
        pDevice.tonMode(inBuffer.data(), inBuffer.count());
        prolParser.reset();
      }
*/      
      pDevice.tonMode(dataPort);
    }else{
      // Device mode (Respond to Attention) handler
      #ifdef USE_RATTN_HANDLER
        if (gpibBus.cfg.cmode == 1) {
          if (gpibBus.isAsserted(ATN_PIN)) {
            atnHandler.attnRequired();
            if (atnHandler.resetParser()) prolParser.reset();
          }
        }
      #endif  // USE_RATTN_HANDLER

    }

  #endif  // USE_PROLOGIX_DEVICE

  #ifdef USE_MACRO_HANDLER_PM
    prolParser.parseMacro(mHandlerPm);
  #endif

#endif  // INTERFACE_PROLOGIX


  delayMicroseconds(10);
}
/***** End of Arduino main loop *****/


