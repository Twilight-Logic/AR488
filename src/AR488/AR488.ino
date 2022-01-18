//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wtype-limits"
//#pragma GCC diagnostic ignored "-Wunused-variable"

#ifdef __AVR__
  #include <avr/wdt.h>
#endif

//#pragma GCC diagnostic pop

#include "AR488_Config.h"
#include "AR488_GPIBbus.h"
#include "AR488_Eeprom.h"


/*
#ifdef USE_INTERRUPTS
  #ifdef __AVR__
    #include <avr/interrupt.h>
  #endif
#endif
*/


/***** Enable EEPROM functions? *****/
/*
#ifdef E2END  // Defined on boards with EEPROM
  #include <EEPROM.h>
#endif
*/

/***** Bluetooth enabled? *****/
#ifdef AR_BT_EN
  #include "AR488_BT.h"
#endif


/***** FWVER "AR488 GPIB controller, ver. 0.51.00, 18/01/2022" *****/
/*
  Arduino IEEE-488 implementation by John Chajecki

  Inspired by the original work of Emanuele Girlando, licensed under a Creative
  Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
  Any code in common with the original work is reproduced here with the explicit
  permission of Emanuele Girlando, who has kindly reviewed and tested this code.

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
   - To receive from the instrument, issue a ++read command or put the controller in auto mode (++auto 1|2)
   - Characters received over the GPIB bus are unbuffered and sent directly to USB
   NOTES:
   - GPIB line in a HIGH state is un-asserted
   - GPIB line in a LOW state is asserted
   - The ATMega processor control pins have a high impedance when set as inputs
   - When set to INPUT_PULLUP, a 10k pull-up (to VCC) resistor is applied to the input
*/

/*
static const char helpText[] PROGMEM = R"EOF(
Standard commands

   ++addr         - display/set device address
   ++auto         - automatically request talk and read response
   ++clr          - send Selected Device Clear to current GPIB address
   ++eoi          - enable/disable assertion of EOI signal
   ++eos          - specify GPIB termination character
   ++eot_enable   - enable/disable appending user specified character to USB
                    output on EOI detection
   ++eot_char     - set character to append to USB output when EOT enabled
   ++ifc          - assert IFC signal for 150 miscoseconds - make AR488
                    controller in charge
   ++llo          - local lockout - disable front panel operation on instrument
   ++loc          - enable front panel operation on instrument
   ++lon          - put controller in listen-only mode (listen to all traffic)
   ++mode         - set the interface mode (0=controller/1=device)
   ++read         - read data from instrument
   ++read_tmo_ms  - read timeout specified between 1 - 3000 milliseconds
   ++rst          - reset the controller
   ++savecfg      - save configration
   ++spoll        - serial poll the addressed host or all instruments
   ++srq          - return status of srq signal (1-asserted/0-not asserted)
   ++status       - set the status byte to be returned on being polled (bit 6 = RQS, i.e SRQ asserted)
   ++trg          - send trigger to selected devices (up to 15 addresses)
   ++ver          - display firmware version

Proprietry commands:

   ++aspoll       - serial poll all instruments (alias: ++spoll all)
   ++default      - set configuration to controller default settings
   ++dcl          - send unaddressed (all) device clear  [power on reset] (is the rst?)
   ++id name      - show/set the name of the interface
   ++id serial    - show/set the serial number of the interface
   ++id verstr    - show/set the version string (replaces setvstr)
   ++idn          - enable/disable reply to *idn? (disabled by default)
   ++ren          - assert or unassert the REN signal
   ++ppoll        - conduct a parallel poll
   ++setvstr      - set custom version string (to identify controller, e.g. "GPIB-USB"). Max 47 chars, excess truncated.
   ++srqauto      - automatically condiuct serial poll when SRQ is asserted
   ++ton          - put controller in talk-only mode (send data only)
   ++verbose      - verbose (human readable) mode
   ++xonxoff
)EOF";
*/

/*
   NOT YET IMPLEMENTED

   ++help     - show summary of commands
   ++myaddr   - set the controller address
*/

/*
   For information regarding the GPIB firmware by Emanualle Girlando see:
   http://egirland.blogspot.com/2014/03/arduino-uno-as-usb-to-gpib-controller.html
*/


/*
   Pin mapping between the Arduino pins and the GPIB connector.
   NOTE:
   GPIB pins 10 and 18-24 are connected to GND
   GPIB pin 12 should be connected to the cable shield (might be n/c)
   Pin mapping follows the layout originally used by Emanuelle Girlando, but adds
   the SRQ line (GPIB 10) on pin 2 and the REN line (GPIB 17) on pin 13. The program
   should therefore be compatible with the original interface design but for full
   functionality will need the remaining two pins to be connected.
   For further information about the AR488 see:
*/


/*********************************/
/***** CONFIGURATION SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvv *****/
// SEE >>>>> Config.h <<<<<
/***** ^^^^^^^^^^^^^^^^^^^^^ *****/
/***** CONFIGURATION SECTION *****/
/*********************************/


/***************************************/
/***** MACRO CONFIGURATION SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
// SEE >>>>> Config.h <<<<<
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** MACRO CONFIGURATION SECTION *****/
/***************************************/


/*************************************/
/***** MACRO STRUCTRURES SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef USE_MACROS

/*** DO NOT MODIFY ***/
/*** vvvvvvvvvvvvv ***/

/***** STARTUP MACRO *****/
const char startup_macro[] PROGMEM = {MACRO_0};

/***** Consts holding USER MACROS 1 - 9 *****/
const char macro_1 [] PROGMEM = {MACRO_1};
const char macro_2 [] PROGMEM = {MACRO_2};
const char macro_3 [] PROGMEM = {MACRO_3};
const char macro_4 [] PROGMEM = {MACRO_4};
const char macro_5 [] PROGMEM = {MACRO_5};
const char macro_6 [] PROGMEM = {MACRO_6};
const char macro_7 [] PROGMEM = {MACRO_7};
const char macro_8 [] PROGMEM = {MACRO_8};
const char macro_9 [] PROGMEM = {MACRO_9};


/* Macro pointer array */
const char * const macros[] PROGMEM = {
  startup_macro,
  macro_1,
  macro_2,
  macro_3,
  macro_4,
  macro_5,
  macro_6,
  macro_7,
  macro_8,
  macro_9
};

/*** ^^^^^^^^^^^^^ ***/
/*** DO NOT MODIFY ***/

#endif
/***** ^^^^^^^^^^^^^^^^^^^^ *****/
/***** MACRO CONFIG SECTION *****/
/********************************/



/**********************************/
/***** SERIAL PORT MANAGEMENT *****/
/***** vvvvvvvvvvvvvvvvvvvvvv *****/

#ifdef AR_SERIAL_ENABLE
  #ifdef AR_SERIAL_TYPE_SW
    #include <SoftwareSerial.h>
    SoftwareSerial AR_SERIAL_PORT(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
  #else
    Stream& dataStream = AR_SERIAL_PORT;
  #endif
#endif

#ifdef DB_SERIAL_ENABLE
  #ifdef DB_SERIAL_TYPE_SW
    #include <SoftwareSerial.h>
    SoftwareSerial DB_SERIAL_PORT(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
  #else
    Stream& debugStream = DB_SERIAL_PORT;
  #endif
#endif

/***** PARSE BUFFERS *****/
/*
 * Note: Ardiono serial input buffer is 64 
 */
// Serial input parsing buffer
static const uint8_t PBSIZE = 128;
char pBuf[PBSIZE];
uint8_t pbPtr = 0;

/***** ^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** SERIAL PORT MANAGEMENT *****/
/**********************************/


/*********************************************/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

// Universal Multiline commands (apply to all devices)

#define GC_LLO 0x11
#define GC_DCL 0x14
#define GC_PPU 0x15
#define GC_SPE 0x18
#define GC_SPD 0x19
#define GC_UNL 0x3F
#define GC_TAD 0x40
#define GC_PPE 0x60
#define GC_PPD 0x70
#define GC_UNT 0x5F
// Address commands
#define GC_LAD 0x20
// Addressed commands
#define GC_GTL 0x01
#define GC_SDC 0x04
#define GC_PPC 0x05
#define GC_GET 0x08

/***** GPIB control states *****/
// Controller mode
#define CINI 0x01 // Controller idle state
#define CIDS 0x02 // Controller idle state
#define CCMS 0x03 // Controller command state
#define CTAS 0x04 // Controller talker active state
#define CLAS 0x05 // Controller listner active state
// Listner/device mode
#define DINI 0x06 // Device initialise state
#define DIDS 0x07 // Device idle state
#define DLAS 0x08 // Device listener active (listening/receiving)
#define DTAS 0x09 // Device talker active (sending) state

/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/*********************************************/



/************************************/
/***** COMMON VARIABLES SECTION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvv *****/

/****** Process status values *****/
#define OK 0
#define ERR 1

/***** Control characters *****/
#define ESC  0x1B   // the USB escape char
#define CR   0xD    // Carriage return
#define LF   0xA    // Newline/linefeed
#define PLUS 0x2B   // '+' character

/***** Controller configuration *****/
/*   
 * Default values set for controller mode
 */
/*
union AR488conf{
  struct{
    bool eot_en;      // Enable/disable append EOT char to string received from GPIB bus before sending to USB
    bool eoi;         // Assert EOI on last data char written to GPIB - 0-disable, 1-enable
    uint8_t cmode;    // Controller/device mode (0=unset, 1=device, 2=controller)
    uint8_t caddr;    // Controller address
    uint8_t paddr;    // Primary device address
    uint8_t saddr;    // Secondary device address
    uint8_t eos;      // EOS (end of send to GPIB) characters [0=CRLF, 1=CR, 2=LF, 3=None]
    uint8_t stat;     // Status byte to return in response to a serial poll
    uint8_t amode;    // Auto mode setting (0=off; 1=Prologix; 2=onquery; 3=continuous);
    int rtmo;         // Read timout (read_tmo_ms) in milliseconds - 0-3000 - value depends on instrument
    char eot_ch;      // EOT character to append to USB output when EOI signal detected
    char vstr[48];    // Custom version string
    uint16_t tmbus;   // Delay to allow the bus control/data lines to settle (1-30,000 microseconds)
    uint8_t eor;      // EOR (end of receive from GPIB instrument) characters [0=CRLF, 1=CR, 2=LF, 3=None, 4=LFCR, 5=ETX, 6=CRLF+ETX, 7=SPACE]
    char sname[16];   // Interface short name
    uint32_t serial;  // Serial number
    uint8_t idn;      // Send ID in response to *idn? 0=disable, 1=send name; 2=send name+serial
  };
  uint8_t db[AR_CFG_SIZE];
};

//struct AR488conf AR488;
union AR488conf AR488;
*/

/****** Global variables with volatile values related to controller state *****/

// GPIB bus object
GPIBbus gpibBus;

// GPIB control state
//uint8_t cstate = 0;

// Verbose mode
bool isVerb = false;

// CR/LF terminated line ready to process
uint8_t lnRdy = 0;      

// GPIB data receive flags
//bool isReading = false;           // Is a GPIB read in progress?
bool addressingSuppressed = false;  // Read with addressing suppressed
bool autoReading = false;           // Auto reading (auto mode 3) GPIB data in progress
bool readWithEoi = false;           // Read eoi requested
bool readWithEndByte = false;       // Read with specified terminator character
bool isQuery = false;               // Direct instrument command is a query
uint8_t tranBrk = 0;                // Transmission break on 1=++, 2=EOI, 3=ATN 4=UNL
uint8_t endByte = 0;                // Termination character

// Device mode - send data
//bool snd = false;

// Escaped character flag
bool isEsc = false;           // Charcter escaped
bool isPlusEscaped = false;   // Plus escaped

// Read only mode flag
bool isRO = false;

// Talk only mode flag
bool isTO = false;

// GPIB command parser
bool aTt = false;
bool aTl = false;

// Data send mode flags
bool dataBufferFull = false;    // Flag when parse buffer is full

// Interrupt flag for MCP23S17
#if defined(AR488_MCP23S17) || defined(AR488_MCP23017)
//extern volatile bool mcpIntA;  // MCP23S17 interrupt handler
//uint8_t mcpPinAssertedReg = 0;
#endif

// State flags set by interrupt being triggered
//extern volatile bool isATN;  // has ATN been asserted?
//extern volatile bool isSRQ;  // has SRQ been asserted?

// SRQ auto mode
bool isSrqa = false;

// Interrupt without handler fired
//volatile bool isBAD = false;

// Whether to run Macro 0 (macros must be enabled)
uint8_t runMacro = 0;

// Send response to *idn?
bool sendIdn = false;

// Xon/Xoff flag (off by default)
//bool xonxoff = false;

/***** ^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** COMMON VARIABLES SECTION *****/
/************************************/



/*******************************/
/***** COMMON CODE SECTION *****/
/***** vvvvvvvvvvvvvvvvvvv *****/


/******  Arduino standard SETUP procedure *****/
void setup() {

  // Disable the watchdog (needed to prevent WDT reset loop)
#ifdef __AVR__
  wdt_disable();
#endif

  // Turn off internal LED (set OUPTUT/LOW)
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif


#ifdef PIN_REMOTE
  pinMode(PIN_REMOTE, OUTPUT);
  digitalWrite(PIN_REMOTE, LOW);
#endif


  // Using MCP23S17 (SPI) expander chip
#ifdef AR488_MCP23S17
  // Enable SPI
  SPI.begin();
  // Optional: Clock divider (slow down the bus speed [optional])
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  // Ensure the MCP select pin is set as an OUPTPUT and is HIGH
  pinMode(MCP_SELECTPIN, OUTPUT);
  digitalWrite(MCP_SELECTPIN, HIGH);
  // Enable the hardware address pins (A0-A2) on the MCP23S17
  mcpByteWrite(MCPCON, 0b00001000);
  // Enable MCP23S17 interrupts
  mcpInterruptsEn();
  // Attach interrupt handler to Arduino board pin for MCP23S17 to signal interrupt has occurred
//  attachInterrupt(digitalPinToInterrupt(MCP_INTERRUPT), mcpIntHandler, FALLING);
#endif


  // Using MCP23017 (I2C) expander chip
#ifdef AR488_MCP23017
  // Start I2C
  Wire.begin();
  // OPtional: Set the I2C bus clock frequency in Hertz (optional - 10000 [slow], 100000 [standard], 400000 [fast], 3400000 [fast plus])
  // Check processor documentation for which modes are supported
//  Wire.setClock(100000);
//  digitalWrite(MCP_SELECTPIN, HIGH);
  // Enable the hardware address pins (A0-A2) on the MCP23017
  mcpByteWrite(MCPCON, 0b00001000);
  // Enable MCP23017 interrupts
  mcpInterruptsEn();
  // Attach interrupt handler to Arduino board pin for MCP23S17 to signal interrupt has occurred
//  attachInterrupt(digitalPinToInterrupt(MCP_INTERRUPT), mcpIntHandler, FALLING);
#endif
  
  
  // Using AVR board with PCINT interrupts
/*
#ifdef USE_INTERRUPTS
  // Turn on interrupts on port
  interruptsEn();
#endif
*/

  // Initialise parse buffer
  flushPbuf();

 // Initialise serial comms over USB or Bluetooth
#ifdef AR_BT_EN
  // Initialise serial at Bluetooth speed rate
  btInit();
  AR_SERIAL_PORT.begin(AR_BT_SPEED);
#else
  // Initialise serial at normal speed rate
  AR_SERIAL_PORT.begin(AR_SERIAL_SPEED);
#endif

// Initialise debug port
#ifdef DB_SERIAL_EN
  DB_SERIAL_PORT.begin(DB_SERIAL_SPEED);
#endif

// Un-comment for diagnostic purposes
/* 
  #if defined(__AVR_ATmega32U4__)
    while(!*arSerial)
    ;
//    Serial.print(F("Starting "));
    for(int i = 0; i < 20; ++i) {  // this gives you 10 seconds to start programming before it crashes
      Serial.print(".");
      delay(500);
    }
    Serial.println("@>");
  #endif // __AVR_ATmega32U4__
*/
// Un-comment for diagnostic purposes

  // Initialise
//  initAR488();

#ifdef E2END
  debugStream.println(F("EEPROM detected!"));
  // Read data from non-volatile memory
  //(will only read if previous config has already been saved)
//  epGetCfg();
  if (!isEepromClear()) {
debugStream.println(F("EEPROM has data."));
    if (!epReadData(gpibBus.cfg.db, GPIB_CFG_SIZE)) {
      // CRC check failed - config data does not match EEPROM
debugStream.println(F("CRC check failed. Erasing EEPROM...."));
      epErase();
      gpibBus.setDefaultCfg();
//      initAR488();
      epWriteData(gpibBus.cfg.db, GPIB_CFG_SIZE);
debugStream.println(F("EEPROM data set to default."));
    }
  }
#endif

  // SN7516x IC support
#ifdef SN7516X
  pinMode(SN7516X_TE, OUTPUT);
  #ifdef SN7516X_DC
    pinMode(SN7516X_DC, OUTPUT);
  #endif
  if (AR488.cmode==2) {
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
    #endif
  }
#endif

  // Start the interface in the configured mode
  gpibBus.begin();

#if defined(USE_MACROS) && defined(RUN_STARTUP)
  // Run startup macro
  execMacro(0);
#endif

#ifdef SAY_HELLO
  dataStream.print(F("AR488 ready "));
  if (gpibBus.isController()){
    dataStream.println(F("(controller)."));
  }else{
    dataStream.println(F("(device)."));
  }
#endif

  Serial.flush();

}
/****** End of Arduino standard SETUP procedure *****/


/***** ARDUINO MAIN LOOP *****/
void loop() {

  bool errFlg = false; 

/*** Macros ***/
/*
 * Run the startup macro if enabled
 */
#ifdef USE_MACROS
  // Run user macro if flagged
  if (runMacro > 0) {
    execMacro(runMacro);
    runMacro = 0;
  }
#endif


/*** Process the buffer ***/
/* Each received char is passed through parser until an un-escaped 
 * CR is encountered. If we have a command then parse and execute.
 * If the line is data (inclding direct instrument commands) then
 * send it to the instrument.
 * NOTE: parseInput() sets lnRdy in serialEvent, readBreak or in the
 * above loop
 * lnRdy=1: process command;
 * lnRdy=2: send data to Gpib
 */

/*
if (lnRdy>0){
  dataStream.print(F("lnRdy: "));
  dataStream.println(lnRdy);
}
*/

  // lnRdy=1: received a command so execute it...
  if (lnRdy == 1) {
    if (autoReading) {
      // Issuing any command stops autoreading
      autoReading = false;
      gpibBus.unAddressDevice();
    }
    execCmd(pBuf, pbPtr);
  }

  // Controller mode:
  if (gpibBus.isController()) {
    // lnRdy=2: received data - send it to the instrument...
    if (lnRdy == 2) {
      sendToInstrument(pBuf, pbPtr);
      // Auto-read data from GPIB bus following any command
      if (gpibBus.cfg.amode == 1) {
        //        delay(10);
        errFlg = gpibBus.receiveData(dataStream, gpibBus.cfg.eoi, false, 0);
      }
      // Auto-receive data from GPIB bus following a query command
      if (gpibBus.cfg.amode == 2 && isQuery) {
        //        delay(10);
        errFlg = gpibBus.receiveData(dataStream, gpibBus.cfg.eoi, false, 0);
        isQuery = false;
      }
    }

    // Check status of SRQ and SPOLL if asserted
//    if (isSRQ && isSrqa) {
/*
    if (gpibBus.isAsserted(SRQ) && isSrqa) {
      spoll_h(NULL);
//      isSRQ = false;
    }
*/
    // Automatic serial poll?
    if (isSrqa) {
      if (gpibBus.isAsserted(SRQ)) spoll_h(NULL);
    }

    // Continuous auto-receive data from GPIB bus
    if ((gpibBus.cfg.amode==3) && autoReading) {
      // Nothing is waiting on the serial input so read data from GPIB
      if (lnRdy==0) {
        errFlg = gpibBus.receiveData(dataStream, readWithEoi, readWithEndByte, endByte);
      }
/*      
      else{
        // Otherwise clear auto-read flag and unaddress device
        autoReading = false;
        gpibBus.unAddressDevice();
      }
*/
    }

    // Did we get an error during read?
    if (errFlg && isVerb) {
      dataStream.println(F("Error while receiving data."));
      errFlg = false;
    }
  }

  // Device mode:
  if (gpibBus.isController()==false) {
    if (isTO) {
      if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
    }else if (isRO) {
      lonMode();
    }else if (gpibBus.isAsserted(ATN)) {
//      dataStream.println(F("Attention signal detected"));
      attnRequired();
//      dataStream.println(F("ATN loop finished"));
    }
/*    
    else{
      if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
    }
*/
  }

  // Reset line ready flag
//  lnRdy = 0;

  // IDN query ?
  if (sendIdn) {
    if (gpibBus.cfg.idn==1) dataStream.println(gpibBus.cfg.sname);
    if (gpibBus.cfg.idn==2) {dataStream.print(gpibBus.cfg.sname);dataStream.print("-");dataStream.println(gpibBus.cfg.serial);}
    sendIdn = false;
  }

  // If charaters waiting in the serial input buffer then call handler
  if (dataStream.available()) lnRdy = serialIn_h();

  delayMicroseconds(5);
}
/***** END MAIN LOOP *****/


/***** Initialise the interface *****/
/*
void initAR488() {
  // Set default values ({'\0'} sets version string array to null)
  gpibBus.cfg = {false, false, 2, 0, 1, 0, 0, 0, 0, 1200, 0, {'\0'}, 0, {'\0'}, 0, 0};
}
*/

/***** Initialise device mode *****/
void initDevice() {
  gpibBus.stop();
  gpibBus.startDeviceMode();
}


/***** Initialise controller mode *****/
void initController() {
  gpibBus.stop();
  gpibBus.startControllerMode();
}


/***** Serial event handler *****/
/*
 * Note: the Arduino serial buffer is 64 characters long. Characters are stored in
 * this buffer until serialEvent_h() is called. parsedInput() takes a character at 
 * a time and places it into the 256 character parse buffer whereupon it is parsed
 * to determine whether a command or data are present.
 * lnRdy=0: terminator not detected yet
 * lnRdy=1: terminator detected, sequence in parse buffer is a ++ command
 * lnRdy=2: terminator detected, sequence in parse buffer is data or direct instrument command
 */ 
uint8_t serialIn_h() {
  uint8_t bufferStatus = 0;
  // Parse serial input until we have detected a line terminator
  while (dataStream.available() && bufferStatus==0) {   // Parse while characters available and line is not complete
    bufferStatus = parseInput(dataStream.read());
  }

#ifdef DEBUG_SERIAL_INPUT
  if (bufferStatus) {
    debugStream.print(F("serialIn_h: bufferStatus: "));
    debugStream.println(bufferStatus);  
  }
#endif

  return bufferStatus;
}


/***** Detect pin state *****/
/*
 * When interrupts are being used the pin state is automatically flagged 
 * when an interrupt is triggered. Where interrupts cannot be used, the
 * state of the pin is read.
 * 
 * Read pin state using digitalRead for Arduino pins or getGpibPinState
 * for MCP23S17 pins.
 */
/* 
bool isAsserted(uint8_t gpibsig) {
#if defined(AR488_MCP23S17) || defined(AR488_MCP23017)
  // Use MCP function to get MCP23S17 or MCP23017 pin state.
  // If interrupt flagged then update mcpPinAssertedReg register
  if (mcpIntA){
//dataStream.println(F("Interrupt flagged - pin state checked"));
    // Clear mcpIntA flag
    mcpIntA = false;
    // Get inverse of pin status at interrupt (0 = true [asserted]; 1 = false [unasserted])
    mcpPinAssertedReg = ~getMcpIntAPinState();
//dataStream.print(F("mcpPinAssertedReg: "));
//dataStream.println(mcpPinAssertedReg, BIN);
  }
  return (mcpPinAssertedReg & (1<<gpibsig));
#else
  // Use digitalRead function to get current Arduino pin state
  return (digitalRead(gpibsig) == LOW) ? true : false;
#endif
}
*/

/*************************************/
/***** Device operation routines *****/
/*************************************/


/***** Unrecognized command *****/
void errBadCmd() {
  dataStream.println(F("Unrecognized command"));
}


/***** Add character to the buffer and parse *****/
uint8_t parseInput(char c) {

  uint8_t r = 0;
/*
  if (xonxoff){
    // Send XOFF when buffer around 80% full
    if (pbPtr < (PBSIZE*0.8)) dataStream.print(0x13);
  }
*/
  // Read until buffer full
  if (pbPtr < PBSIZE) {
    if (isVerb && c!=LF) dataStream.print(c);  // Humans like to see what they are typing...
    // Actions on specific characters
    switch (c) {
      // Carriage return or newline? Then process the line
      case CR:
      case LF:
        // If escaped just add to buffer
        if (isEsc) {
          addPbuf(c);
          isEsc = false;
        } else {
          // Carriage return on blank line?
          // Note: for data CR and LF will always be escaped
          if (pbPtr == 0) {
            flushPbuf();
            if (isVerb) {
              dataStream.println();
              showPrompt();
            }
            return 0;
          } else {
//            if (isVerb) dataStream.println();  // Move to new line
#ifdef DEBUG_SERIAL_INPUT
            debugStream.print(F("parseInput: Received ")); debugStream.println(pBuf);
#endif
            // Buffer starts with ++ and contains at least 3 characters - command?
            if (pbPtr>2 && isCmd(pBuf) && !isPlusEscaped) {
              // Exclamation mark (break read loop command)
              if (pBuf[2]==0x21) {
                r = 3;
                flushPbuf();
              // Otherwise flag command received and ready to process 
              }else{
                r = 1;
              }
            // Buffer contains *idn? query and interface to respond
            }else if (pbPtr>3 && gpibBus.cfg.idn>0 && isIdnQuery(pBuf)){
              sendIdn = true;
              flushPbuf();
            // Buffer has at least 1 character = instrument data to send to gpib bus
            }else if (pbPtr > 0) {
              r = 2;
            }
            isPlusEscaped = false;
#ifdef DEBUG_SERIAL_INPUT
            debugStream.print(F("R: "));debugStream.println(r);
#endif
//            return r;
          }
        }
        break;
      case ESC:
        // Handle the escape character
        if (isEsc) {
          // Add character to buffer and cancel escape
          addPbuf(c);
          isEsc = false;
        } else {
          // Set escape flag
          isEsc  = true;  // Set escape flag
        }
        break;
      case PLUS:
        if (isEsc) {
          isEsc = false;
          if (pbPtr < 2) isPlusEscaped = true;
        }
        addPbuf(c);
//        if (isVerb) dataStream.print(c);
        break;
      // Something else?
      default: // any char other than defined above
//        if (isVerb) dataStream.print(c);  // Humans like to see what they are typing...
        // Buffer contains '++' (start of command). Stop sending data to serial port by halting GPIB receive.
        addPbuf(c);
        isEsc = false;
    }
  }
  if (pbPtr >= PBSIZE) {
    if (isCmd(pBuf) && !r) {  // Command without terminator and buffer full
      if (isVerb) {
        dataStream.println(F("ERROR - Command buffer overflow!"));
      }
      flushPbuf();
    }else{  // Buffer contains data and is full, so process the buffer (send data via GPIB)
      dataBufferFull = true;
      // Signal to GPIB object that more data will follow (suppress GPIB addressing)
//      gpibBus.setDataContinuity(true);
      r = 2;
    }
  }
  return r;
}


/***** Is this a command? *****/
bool isCmd(char *buffr) {
  if (buffr[0] == PLUS && buffr[1] == PLUS) {
#ifdef DEBUG_SERIAL_INPUT
    debugStream.println(F("isCmd: Command detected."));
#endif
    return true;
  }
  return false;
}


/***** Is this an *idn? query? *****/
bool isIdnQuery(char *buffr) {
  // Check for upper or lower case *idn?
  if (strncasecmp(buffr, "*idn?", 5)==0) {
#ifdef DEBUG_SERIAL_INPUT
    debugStream.println(F("isIdnQuery: Detected IDN query."));
#endif
    return true;
  }
  return false;
}


/***** ++read command detected? *****/
bool isRead(char *buffr) {
  char cmd[4];
  // Copy 2nd to 5th character
  for (int i = 2; i < 6; i++) {
    cmd[i - 2] = buffr[i];
  }
  // Compare with 'read'
  if (strncmp(cmd, "read", 4) == 0) return true;
  return false;
}


/***** Add character to the buffer *****/
void addPbuf(char c) {
  pBuf[pbPtr] = c;
  pbPtr++;
}


/***** Clear the parse buffer *****/
void flushPbuf() {
  memset(pBuf, '\0', PBSIZE);
  pbPtr = 0;
}


/***** Comand function record *****/
struct cmdRec { 
  const char* token; 
  int opmode;
  void (*handler)(char *);
};


/***** Array containing index of accepted ++ commands *****/
/*
 * Commands without parameters require casting to a pointer
 * requiring a char* parameter. The functon is called with
 * NULL by the command processor.
 * 
 * Format: token, mode, function_ptr
 * Mode: 1=device; 2=controller; 3=both; 
 */
static cmdRec cmdHidx [] = { 
 
  { "addr",        3, addr_h      }, 
  { "allspoll",    2, (void(*)(char*)) aspoll_h  },
  { "auto",        2, amode_h     },
  { "clr",         2, (void(*)(char*)) clr_h     },
  { "dcl",         2, (void(*)(char*)) dcl_h     },
  { "default",     3, (void(*)(char*)) default_h },
  { "eoi",         3, eoi_h       },
  { "eor",         3, eor_h       },
  { "eos",         3, eos_h       },
  { "eot_char",    3, eot_char_h  },
  { "eot_enable",  3, eot_en_h    },
  { "ifc",         2, (void(*)(char*)) ifc_h     },
  { "id",          3, id_h        },
  { "idn",         3, idn_h       },
  { "llo",         2, llo_h       },
  { "loc",         2, loc_h       },
  { "lon",         1, lon_h       },
  { "macro",       2, macro_h     },
  { "mla",         2, (void(*)(char*)) sendmla_h },
  { "mode" ,       3, cmode_h     },
  { "msa",         2, sendmsa_h   },
  { "mta",         2, (void(*)(char*)) sendmta_h },
  { "ppoll",       2, (void(*)(char*)) ppoll_h   },
  { "read",        2, read_h      },
  { "read_tmo_ms", 2, rtmo_h      },
  { "ren",         2, ren_h       },
  { "repeat",      2, repeat_h    },
  { "rst",         3, (void(*)(char*)) rst_h     },
  { "trg",         2, trg_h       },
  { "savecfg",     3, (void(*)(char*)) save_h    },
  { "setvstr",     3, setvstr_h   },
  { "spoll",       2, spoll_h     },
  { "srq",         2, (void(*)(char*)) srq_h     },
  { "srqauto",     2, srqa_h      },
  { "status",      1, stat_h      },
  { "ton",         1, ton_h       },
  { "unl",         2, (void(*)(char*)) unlisten_h  },
  { "unt",         2, (void(*)(char*)) untalk_h    },
  { "ver",         3, ver_h       },
  { "verbose",     3, (void(*)(char*)) verb_h    },
//  { "tmbus",       3, tmbus_h     },
  { "xdiag",       3, xdiag_h     }
//  { "xonxoff",     3, xonxoff_h   }
};


/***** Show a prompt *****/
void showPrompt() {
  // Print a prompt
  dataStream.print("> ");
}


/****** Send data to instrument *****/
/* Processes the parse buffer whenever a full CR or LF
 * and sends data to the instrument
 */
void sendToInstrument(char *buffr, uint8_t dsize) {

#ifdef DEBUG_SEND_TO_INSTR
  if (buffr[dsize] != LF) debugStream.println();
  debugStream.print(F("sendToInstrument: Received for sending: "));
  printHex(buffr, dsize);
#endif

  // Is this an instrument query command (string ending with ?)
  if (buffr[dsize-1] == '?') isQuery = true;

  // Address device
  if (!gpibBus.isDeviceAddressed()) gpibBus.addressDevice(gpibBus.cfg.paddr, LISTEN);

  // Send string to instrument
  gpibBus.sendData(buffr, dsize);

  // Address device
  if (dataBufferFull) {
    dataBufferFull = false;
  }else{
    gpibBus.unAddressDevice();
  }

#ifdef DEBUG_SEND_TO_INSTR
  debugStream.println(F("sendToInstrument: done."));
#endif

  // Show a prompt on completion?
  if (isVerb) showPrompt();

  // Flush the parse buffer
  flushPbuf();
  lnRdy = 0;
}


/***** Execute a command *****/
void execCmd(char *buffr, uint8_t dsize) {
  char line[PBSIZE];

  // Copy collected chars to line buffer
  memcpy(line, buffr, dsize);

  // Flush the parse buffer
  flushPbuf();
  lnRdy = 0;

#ifdef DEBUG_CMD_PARSER
  debugStream.print(F("execCmd: command received: ")); printHex(line, dsize);
#endif

  // Its a ++command so shift everything two bytes left (ignore ++) and parse
  for (int i = 0; i < dsize-2; i++) {
    line[i] = line[i + 2];
  }
  // Replace last two bytes with a null (\0) character
  line[dsize - 2] = '\0';
  line[dsize - 1] = '\0';
#ifdef DEBUG_CMD_PARSER
  debugStream.print(F("execCmd: sent to command processor: ")); printHex(line, dsize-2);
#endif
  // Execute the command
  if (isVerb) dataStream.println(); // Shift output to next line
  getCmd(line);

  // Show a prompt on completion?
  if (isVerb) showPrompt();
}


/***** Extract command and pass to handler *****/
void getCmd(char *buffr) {

  char *token;  // Pointer to command token
  char *params; // Pointer to parameters (remaining buffer characters)
  
  int casize = sizeof(cmdHidx) / sizeof(cmdHidx[0]);
  int i = 0;

#ifdef DEBUG_CMD_PARSER
  debugStream.print("getCmd: ");
  debugStream.print(buffr); debugStream.print(F(" - length: ")); debugStream.println(strlen(buffr));
#endif

  // If terminator on blank line then return immediately without processing anything 
  if (buffr[0] == 0x00) return;
  if (buffr[0] == CR) return;
  if (buffr[0] == LF) return;

  // Get the first token
  token = strtok(buffr, " \t");

#ifdef DEBUG_CMD_PARSER
  debugStream.print("getCmd: process token: "); debugStream.println(token);
#endif

  // Check whether it is a valid command token
  i = 0;
  do {
    if (strcasecmp(cmdHidx[i].token, token) == 0) break;
    i++;
  } while (i < casize);

  if (i < casize) {
    // We have found a valid command and handler
#ifdef DEBUG_CMD_PARSER
    debugStream.print("getCmd: found handler for: "); debugStream.println(cmdHidx[i].token);
#endif
    // If command is relevant to mode then execute it
    if (cmdHidx[i].opmode & gpibBus.cfg.cmode) {
      // If its a command with parameters
      // Copy command parameters to params and call handler with parameters
      params = token + strlen(token) + 1;
  
      // If command parameters were specified
      if (strlen(params) > 0) {
#ifdef DEBUG_CMD_PARSER
        debugStream.print(F("getCmd: calling handler with parameters: "));
        debugStream.println(params);
#endif
        // Call handler with parameters specified
        cmdHidx[i].handler(params);
      }else{
#ifdef DEBUG_CMD_PARSER
        debugStream.println(F("getCmd: calling handler without parameters..."));
#endif
        // Call handler without parameters
        cmdHidx[i].handler(NULL);
      }
#ifdef DEBUG_CMD_PARSER
      debugStream.println(F("getCmd: handler done."));
#endif
    }else{
      errBadCmd();
      if (isVerb) dataStream.println(F("getCmd: command not available in this mode."));
    }
  } else {
    // No valid command found
    errBadCmd();
  }
 
}


/***** Prints charaters as hex bytes *****/
void printHex(char *buffr, int dsize) {
#ifdef DB_SERIAL_ENABLE
  for (int i = 0; i < dsize; i++) {
    debugStream.print(buffr[i], HEX); debugStream.print(" ");
  }
  debugStream.println();
#endif
}


/***** Check whether a parameter is in range *****/
/* Convert string to integer and check whether value is within
 * lowl to higl inclusive. Also returns converted text in param
 * to a uint16_t integer in rval. Returns true if successful, 
 * false if not
*/
bool notInRange(char *param, uint16_t lowl, uint16_t higl, uint16_t &rval) {

  // Null string passed?
  if (strlen(param) == 0) return true;

  // Convert to integer
  rval = 0;
  rval = atoi(param);

  // Check range
  if (rval < lowl || rval > higl) {
    errBadCmd();
    if (isVerb) {
      dataStream.print(F("Valid range is between "));
      dataStream.print(lowl);
      dataStream.print(F(" and "));
      dataStream.println(higl);
    }
    return true;
  }
  return false;
}


/***** If enabled, executes a macro *****/
#ifdef USE_MACROS
void execMacro(uint8_t idx) {
  char c;
  const char * macro = pgm_read_word(macros + idx);
  int ssize = strlen_P(macro);

  // Read characters from macro character array
  for (int i = 0; i < ssize; i++) {
    c = pgm_read_byte_near(macro + i);
    if (c == CR || c == LF || i == (ssize - 1)) {
      // Reached terminator or end of marcro. Add to buffer before processing
      if (i == ssize-1) {
        // Check buffer and add character
        if (pbPtr < (PBSIZE - 1)){
          if ((c != CR) && (c != LF)) addPbuf(c);
        }else{
          // Buffer full - clear and exit
          flushPbuf();
          return;
        }
      }
      if (isCmd(pBuf)){
        execCmd(pBuf, strlen(pBuf));
      }else{
        sendToInstrument(pBuf, strlen(pBuf));
      }
      // Done - clear the buffer
      flushPbuf();
    } else {
      // Check buffer and add character
      if (pbPtr < (PBSIZE - 1)) {
        addPbuf(c);
      } else {
        // Exceeds buffer size - clear buffer and exit
        i = ssize;
        return;
      }
    }
  }

  // Clear the buffer ready for serial input
  flushPbuf();
}
#endif


/*************************************/
/***** STANDARD COMMAND HANDLERS *****/
/*************************************/

/***** Show or change device address *****/
void addr_h(char *params) {
  //  char *param, *stat;
//  char *param;
  uint16_t val;
  if (params != NULL) {

    // Primary address
//    param = strtok(params, " \t");
//    if (notInRange(param, 1, 30, val)) return;
    if (notInRange(params, 1, 30, val)) return;
    if (val == gpibBus.cfg.caddr) {
      errBadCmd();
      if (isVerb) dataStream.println(F("That is my address! Address of a remote device is required."));
      return;
    }
    gpibBus.cfg.paddr = val;
    if (isVerb) {
      dataStream.print(F("Set device primary address to: "));
      dataStream.println(val);
    }
/*
    // Secondary address
    gpibBus.cfg.saddr = 0;
    val = 0;
    param = strtok(NULL, " \t");
    if (param != NULL) {
      if (notInRange(param, 96, 126, val)) return;
      gpibBus.cfg.saddr = val;
      if (isVerb) {
        dataStream.print("Set device secondary address to: ");
        dataStream.println(val);
      }
    }
*/
  } else {
//    dataStream.print(gpibBus.cfg.paddr);
    dataStream.println(gpibBus.cfg.paddr);
/*    
    if (gpibBus.cfg.saddr > 0) {
      dataStream.print(F(" "));
      dataStream.print(gpibBus.cfg.saddr);
    }
    dataStream.println();
*/
  }
}


/***** Show or set read timout *****/
void rtmo_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 1, 32000, val)) return;
    gpibBus.cfg.rtmo = val;
    if (isVerb) {
      dataStream.print(F("Set [read_tmo_ms] to: "));
      dataStream.print(val);
      dataStream.println(F(" milliseconds"));
    }
  } else {
    dataStream.println(gpibBus.cfg.rtmo);
  }
}


/***** Show or set end of send character *****/
void eos_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 3, val)) return;
    gpibBus.cfg.eos = (uint8_t)val;
    if (isVerb) {
      dataStream.print(F("Set EOS to: "));
      dataStream.println(val);
    };
  } else {
    dataStream.println(gpibBus.cfg.eos);
  }
}


/***** Show or set EOI assertion on/off *****/
void eoi_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    gpibBus.cfg.eoi = val ? true : false;
    if (isVerb) {
      dataStream.print(F("Set EOI assertion: "));
      dataStream.println(val ? "ON" : "OFF");
    };
  } else {
    dataStream.println(gpibBus.cfg.eoi);
  }
}


/***** Show or set interface to controller/device mode *****/
void cmode_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    switch (val) {
      case 0:
        gpibBus.startDeviceMode();
        break;
      case 1:
        gpibBus.startControllerMode();
        break;
    }
    if (isVerb) {
      dataStream.print(F("Interface mode set to: "));
      dataStream.println(val ? "CONTROLLER" : "DEVICE");
    }
  } else {
    dataStream.println(gpibBus.isController());
  }
}


/***** Show or enable/disable sending of end of transmission character *****/
void eot_en_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    gpibBus.cfg.eot_en = val ? true : false;
    if (isVerb) {
      dataStream.print(F("Appending of EOT character: "));
      dataStream.println(val ? "ON" : "OFF");
    }
  } else {
    dataStream.println(gpibBus.cfg.eot_en);
  }
}


/***** Show or set end of transmission character *****/
void eot_char_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 255, val)) return;
    gpibBus.cfg.eot_ch = (uint8_t)val;
    if (isVerb) {
      dataStream.print(F("EOT set to ASCII character: "));
      dataStream.println(val);
    };
  } else {
    dataStream.println(gpibBus.cfg.eot_ch, DEC);
  }
}


/***** Show or enable/disable auto mode *****/
void amode_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 3, val)) return;
    if (val > 0 && isVerb) {
      dataStream.println(F("WARNING: automode ON can cause some devices to generate"));
      dataStream.println(F("         'addressed to talk but nothing to say' errors"));
    }
    gpibBus.cfg.amode = (uint8_t)val;
    if (gpibBus.cfg.amode < 3) autoReading = false;
    if (isVerb) {
      dataStream.print(F("Auto mode: "));
      dataStream.println(gpibBus.cfg.amode);
    }
  } else {
    dataStream.println(gpibBus.cfg.amode);
  }
}


/***** Display the controller version string *****/
void ver_h(char *params) {
  // If "real" requested
  if (params != NULL && strncasecmp(params, "real", 3) == 0) {
    dataStream.println(F(FWVER));
    // Otherwise depends on whether we have a custom string set
  } else {
    if (strlen(gpibBus.cfg.vstr) > 0) {
      dataStream.println(gpibBus.cfg.vstr);
    } else {
      dataStream.println(F(FWVER));
    }
  }
}


/***** Address device to talk and read the sent data *****/
void read_h(char *params) {
  // Clear read flags
  readWithEoi = false;
  readWithEndByte = false;
  endByte = 0;
  // Read any parameters
  if (params != NULL) {
    if (strlen(params) > 3) {
      if (isVerb) dataStream.println(F("Invalid parameter - ignored!"));
    } else if (strncasecmp(params, "eoi", 3) == 0) { // Read with eoi detection
      readWithEoi = true;
/*
    } else if (strncasecmp(params, "nae", 3) == 0) { // Read with no addressing and with eoi detection
      readWithEoi = true;
      suppressAddressing = true;
    } else if (strncasecmp(params, "nad", 3) == 0) { // Read with no addressing
      readWithEoi = false;
      suppressAddressing = true;
*/
    } else { // Assume ASCII character given and convert to an 8 bit byte
      readWithEndByte = true;
      endByte = atoi(params);
    }
  }

debugStream.print(F("readWithEoi:     "));
debugStream.println(readWithEoi);
debugStream.print(F("readWithEndByte: "));
debugStream.println(readWithEndByte);

  
  if (gpibBus.cfg.amode == 3) {
    // In auto continuous mode we set this flag to indicate we are ready for continuous read
    autoReading = true;
    if (!gpibBus.isDeviceAddressed()) gpibBus.addressDevice(gpibBus.cfg.paddr, TALK);
  } else {
    // If auto mode is disabled we do a single read
    if ((!addressingSuppressed) && (!gpibBus.isDeviceAddressed())) gpibBus.addressDevice(gpibBus.cfg.paddr, TALK);
    gpibBus.receiveData(dataStream, readWithEoi, readWithEndByte, endByte);
    if (!addressingSuppressed) gpibBus.unAddressDevice();
  }
  if (addressingSuppressed) addressingSuppressed = false;
}


/***** Send device clear (usually resets the device to power on state) *****/
void clr_h() {
  if (gpibBus.sendSDC())  {
    if (isVerb) dataStream.println(F("Failed to send SDC"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Send local lockout command *****/
void llo_h(char *params) {
  // NOTE: REN *MUST* be asserted (LOW)
  if (digitalRead(REN)==LOW) {
    // For 'all' send LLO to the bus without addressing any device
    // Devices will show REM as soon as they are addressed and need to be released with LOC
    if (params != NULL) {
      if (0 == strncmp(params, "all", 3)) {
        if (gpibBus.sendCmd(GC_LLO)) {
          if (isVerb) dataStream.println(F("Failed to send universal LLO."));
        }
      }
    } else {
      // Send LLO to currently addressed device
      if (gpibBus.sendLLO()){
        if (isVerb) dataStream.println(F("Failed to send LLO!"));
      }
    }
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Send Go To Local (GTL) command *****/
void loc_h(char *params) {
  // REN *MUST* be asserted (LOW)
  if (digitalRead(REN)==LOW) {
    if (params != NULL) {
      if (strncmp(params, "all", 3) == 0) {
        // Send request to clear all devices to local
        gpibBus.sendAllClear();
      }
    } else {
      // Send GTL to addressed device
      if (gpibBus.sendGTL()) {
        if (isVerb) dataStream.println(F("Failed to send LOC!"));
      }
      // Set GPIB controls back to idle state
      gpibBus.setControls(CIDS);
    }
  }
}


/***** Assert IFC for 150 microseconds *****/
/* This indicates that the AR488 the Controller-in-Charge on
 * the bus and causes all interfaces to return to their idle 
 * state
 */
void ifc_h() {
  if (gpibBus.cfg.cmode==2) {
    // Assert IFC
    gpibBus.setControlVal(0b00000000, 0b00000001, 0);
    delayMicroseconds(150);
    // De-assert IFC
    gpibBus.setControlVal(0b00000001, 0b00000001, 0);
    if (isVerb) dataStream.println(F("IFC signal asserted for 150 microseconds"));
  }
}


/***** Send a trigger command *****/
void trg_h(char *params) {
  char *param;
  uint8_t addrs[15] = {0};
  uint16_t val = 0;
  uint8_t cnt = 0;

  addrs[0] = addrs[0]; // Meaningless as both are zero but defaults compiler warning!

  // Read parameters
  if (params == NULL) {
    // No parameters - trigger addressed device only
    addrs[0] = gpibBus.cfg.paddr;
    cnt++;
  } else {
    // Read address parameters into array
    while (cnt < 15) {
      if (cnt == 0) {
        param = strtok(params, " \t");
      } else {
        param = strtok(NULL, " \t");
      }
      if (param == NULL) {
        break;  // Stop when there are no more parameters
      }else{    
        if (notInRange(param, 1, 30, val)) return;
        addrs[cnt] = (uint8_t)val;
        cnt++;
      }
    }
  }

  // If we have some addresses to trigger....
  if (cnt > 0) {
    for (int i = 0; i < cnt; i++) {
      // Sent GET to the requested device
      if (gpibBus.sendGET(addrs[i]))  {
        if (isVerb) dataStream.println(F("Failed to trigger device!"));
        return;
      }
    }

    // Set GPIB controls back to idle state
    gpibBus.setControls(CIDS);

    if (isVerb) dataStream.println(F("Group trigger completed."));
  }
}


/***** Reset the controller *****/
/*
 * Arduinos can use the watchdog timer to reset the MCU
 * For other devices, we restart the program instead by
 * jumping to address 0x0000. This is not a hardware reset
 * and will not reset a crashed MCU, but it will re-start
 * the interface program and re-initialise all parameters. 
 */
void rst_h() {
#ifdef WDTO_1S
  // Where defined, reset controller using watchdog timeout
  unsigned long tout;
  tout = millis() + 2000;
  wdt_enable(WDTO_1S);
  while (millis() < tout) {};
  // Should never reach here....
  if (isVerb) {
    dataStream.println(F("Reset FAILED."));
  };
#else
  // Otherwise restart program (soft reset)
  asm volatile ("  jmp 0");
#endif
}


/***** Serial Poll Handler *****/
void spoll_h(char *params) {
  char *param;
  uint8_t addrs[15];
  uint8_t sb = 0;
  uint8_t r;
  //  uint8_t i = 0;
  uint8_t j = 0;
  uint16_t val = 0;
  bool all = false;
  bool eoiDetected = false;

  // Initialise address array
  for (int i = 0; i < 15; i++) {
    addrs[i] = 0;
  }

  // Read parameters
  if (params == NULL) {
    // No parameters - trigger addressed device only
    addrs[0] = gpibBus.cfg.paddr;
    j = 1;
  } else {
    // Read address parameters into array
    while (j < 15) {
      if (j == 0) {
        param = strtok(params, " \t");
      } else {
        param = strtok(NULL, " \t");
      }
      // The 'all' parameter given?
      if (strncmp(param, "all", 3) == 0) {
        all = true;
        j = 30;
        if (isVerb) dataStream.println(F("Serial poll of all devices requested..."));
        break;
        // Read all address parameters
      } else if (strlen(params) < 3) { // No more than 2 characters
        if (notInRange(param, 1, 30, val)) return;
        addrs[j] = (uint8_t)val;
        j++;
      } else {
        errBadCmd();
        if (isVerb) dataStream.println(F("Invalid parameter"));
        return;
      }
    }
  }

  // Send Unlisten [UNL] to all devices
  if ( gpibBus.sendCmd(GC_UNL) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send UNL"));
#endif
    return;
  }

  // Controller addresses itself as listner
  if ( gpibBus.sendCmd(GC_LAD + gpibBus.cfg.caddr) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send LAD"));
#endif
    return;
  }

  // Send Serial Poll Enable [SPE] to all devices
  if ( gpibBus.sendCmd(GC_SPE) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send SPE"));
#endif
    return;
  }

  // Poll GPIB address or addresses as set by i and j
  for (int i = 0; i < j; i++) {

    // Set GPIB address in val
    if (all) {
      val = i;
    } else {
      val = addrs[i];
    }

    // Don't need to poll own address
    if (val != gpibBus.cfg.caddr) {

      // Address a device to talk
      if ( gpibBus.sendCmd(GC_TAD + val) )  {

#ifdef DEBUG_SPOLL
        debugStream.println(F("spoll_h: failed to send TAD"));
#endif
        return;
      }

      // Set GPIB control to controller active listner state (ATN unasserted)
      gpibBus.setControls(CLAS);

      // Read the response byte (usually device status) using handshake - suppress EOI detection
      r = gpibBus.readByte(&sb, false, &eoiDetected);

      // If we successfully read a byte
      if (!r) {
        if (j > 1) {
          // If all, return specially formatted response: SRQ:addr,status
          // but only when RQS bit set
          if (sb & 0x40) {
            dataStream.print(F("SRQ:")); dataStream.print(i); dataStream.print(F(",")); dataStream.println(sb, DEC);
            i = j;
          }
        } else {
          // Return decimal number representing status byte
          dataStream.println(sb, DEC);
          if (isVerb) {
            dataStream.print(F("Received status byte ["));
            dataStream.print(sb);
            dataStream.print(F("] from device at address: "));
            dataStream.println(val);
          }
          i = j;
        }
      } else {
        if (isVerb) dataStream.println(F("Failed to retrieve status byte"));
      }
    }
  }
  if (all) dataStream.println();

  // Send Serial Poll Disable [SPD] to all devices
  if ( gpibBus.sendCmd(GC_SPD) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send SPD"));
#endif
    return;
  }

  // Send Untalk [UNT] to all devices
  if ( gpibBus.sendCmd(GC_UNT) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send UNT"));
#endif
    return;
  }

  // Unadress listners [UNL] to all devices
  if ( gpibBus.sendCmd(GC_UNL) )  {
#ifdef DEBUG_SPOLL
    debugStream.println(F("spoll_h: failed to send UNL"));
#endif
    return;
  }

  // Set GPIB control to controller idle state
  gpibBus.setControls(CIDS);

  // Set SRQ to status of SRQ line. Should now be unasserted but, if it is
  // still asserted, then another device may be requesting service so another
  // serial poll will be called from the main loop
/*  FLAG NO LONGER USED = NOW CHECKING STATUS OF LINE
  if (digitalRead(SRQ) == LOW) {
    isSRQ = true;
  } else {
    isSRQ = false;
  }
*/
  if (isVerb) dataStream.println(F("Serial poll completed."));

}


/***** Return status of SRQ line *****/
void srq_h() {
  //NOTE: LOW=0=asserted, HIGH=1=unasserted
//  dataStream.println(!digitalRead(SRQ));
  dataStream.println(gpibBus.isAsserted(SRQ));
}


/***** Set the status byte (device mode) *****/
void stat_h(char *params) {
  uint16_t statusByte = 0;
  // A parameter given?
  if (params != NULL) {
    // Byte value given?
    if (notInRange(params, 0, 255, statusByte)) return;
    gpibBus.setStatus((uint8_t)statusByte);
  } else {
    // Return the currently set status byte
    dataStream.println(gpibBus.cfg.stat);
  }
}


/***** Save controller configuration *****/
void save_h() {
#ifdef E2END
  epWriteData(gpibBus.cfg.db, GPIB_CFG_SIZE);
  if (isVerb) dataStream.println(F("Settings saved."));
#else
  dataStream.println(F("EEPROM not supported."));
#endif
}


/***** Show state or enable/disable listen only mode *****/
void lon_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    isRO = val ? true : false;
    if (isTO) isTO = false; // Talk-only mode must be disabled!
    if (isVerb) {
      dataStream.print(F("LON: "));
      dataStream.println(val ? "ON" : "OFF") ;
    }
  } else {
    dataStream.println(isRO);
  }
}


/***********************************/
/***** CUSTOM COMMAND HANDLERS *****/
/***********************************/

/***** All serial poll *****/
/*
 * Polls all devices, not just the currently addressed instrument
 * This is an alias wrapper for ++spoll all
 */
void aspoll_h() {
  //  char all[4];
  //  strcpy(all, "all\0");
  spoll_h((char*)"all");
}


/***** Send Universal Device Clear *****/
/*
 * The universal Device Clear (DCL) is unaddressed and affects all devices on the Gpib bus.
 */
void dcl_h() {
  if ( gpibBus.sendCmd(GC_DCL) )  {
    if (isVerb) dataStream.println(F("Sending DCL failed"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
}


/***** Re-load default configuration *****/
void default_h() {
  gpibBus.setDefaultCfg();
}


/***** Show or set end of receive character(s) *****/
void eor_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 15, val)) return;
    gpibBus.cfg.eor = (uint8_t)val;
    if (isVerb) {
      dataStream.print(F("Set EOR to: "));
      dataStream.println(val);
    };
  } else {
    if (gpibBus.cfg.eor>7) gpibBus.cfg.eor = 0;  // Needed to reset FF read from EEPROM after FW upgrade
    dataStream.println(gpibBus.cfg.eor);
  }
}


/***** Parallel Poll Handler *****/
/*
 * Device must be set to respond on DIO line 1 - 8
 */
void ppoll_h() {
  uint8_t sb = 0;

  // Poll devices
  // Start in controller idle state
  gpibBus.setControls(CIDS);
  delayMicroseconds(20);
  // Assert ATN and EOI
  gpibBus.setControlVal(0b00000000, 0b10010000, 0);
  //  setGpibState(0b10010000, 0b00000000, 0b10010000);
  delayMicroseconds(20);
  // Read data byte from GPIB bus without handshake
  sb = readGpibDbus();
  // Return to controller idle state (ATN and EOI unasserted)
  gpibBus.setControls(CIDS);

  // Output the response byte
  dataStream.println(sb, DEC);

  if (isVerb) dataStream.println(F("Parallel poll completed."));
}


/***** Assert or de-assert REN 0=de-assert; 1=assert *****/
void ren_h(char *params) {
#if defined (SN7516X) && not defined (SN7516X_DC)
  params = params;
  dataStream.println(F("Unavailable")) ;
#else
  // char *stat;
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    digitalWrite(REN, (val ? LOW : HIGH));
    if (isVerb) {
      dataStream.print(F("REN: "));
      dataStream.println(val ? "REN asserted" : "REN un-asserted") ;
    };
  } else {
    dataStream.println(digitalRead(REN) ? 0 : 1);
  }
#endif
}


/***** Enable verbose mode 0=OFF; 1=ON *****/
void verb_h() {
  isVerb = !isVerb;
  dataStream.print("Verbose: ");
  dataStream.println(isVerb ? "ON" : "OFF");
}


/***** Set version string *****/
/* Replace the standard AR488 version string with something else
 *  NOTE: some instrument software requires a sepcific version string to ID the interface
 */
void setvstr_h(char *params) {
  uint8_t plen;
  char idparams[64];
  plen = strlen(params);
  memset(idparams, '\0', 64);
  strncpy(idparams, "verstr ", 7);
  if (plen>47) plen = 47; // Ignore anything over 47 characters
  strncat(idparams, params, plen);

  id_h(idparams);
  
/*  
  if (params != NULL) {
    len = strlen(params);
    if (len>47) len=47; 
    memset(AR488.vstr, '\0', 48);
    strncpy(AR488.vstr, params, len);
    if (isVerb) {
      dataStream.print(F("Changed version string to: "));
      dataStream.println(params);
    };
  }
*/  
}


/***** Talk only mode *****/
void ton_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    isTO = val ? true : false;
    if (isTO) isRO = false; // Read-only mode must be disabled in TO mode!
    if (isVerb) {
      dataStream.print(F("TON: "));
      dataStream.println(val ? "ON" : "OFF") ;
    }
  } else {
    dataStream.println(isTO);
  }
}


/***** SRQ auto - show or enable/disable automatic spoll on SRQ *****/
/*
 * In device mode, when the SRQ interrupt is triggered and SRQ
 * auto is set to 1, a serial poll is conducted automatically
 * and the status byte for the instrument requiring service is
 * automatically returned. When srqauto is set to 0 (default)
 * an ++spoll command needs to be given manually to return
 * the status byte.
 */
void srqa_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    switch (val) {
      case 0:
        isSrqa = false;
        break;
      case 1:
        isSrqa = true;
        break;
    }
    if (isVerb) dataStream.println(isSrqa ? "SRQ auto ON" : "SRQ auto OFF") ;
  } else {
    dataStream.println(isSrqa);
  }
}


/***** Repeat a given command and return result *****/
void repeat_h(char *params) {

  uint16_t count;
  uint16_t tmdly;
  char *param;

  if (params != NULL) {
    // Count (number of repetitions)
    param = strtok(params, " \t");
    if (strlen(param) > 0) {
      if (notInRange(param, 2, 255, count)) return;
    }
    // Time delay (milliseconds)
    param = strtok(NULL, " \t");
    if (strlen(param) > 0) {
      if (notInRange(param, 0, 30000, tmdly)) return;
    }

    // Pointer to remainder of parameters string
    param = strtok(NULL, "\n\r");
    if (strlen(param) > 0) {
      for (uint16_t i = 0; i < count; i++) {
        // Send string to instrument
        gpibBus.sendData(param, strlen(param));
        delay(tmdly);
        gpibBus.receiveData(dataStream, gpibBus.cfg.eoi, false, 0);
      }
    } else {
      errBadCmd();
      if (isVerb) dataStream.println(F("Missing parameter"));
      return;
    }
  } else {
    errBadCmd();
    if (isVerb) dataStream.println(F("Missing parameters"));
  }

}


/***** Run a macro *****/
void macro_h(char *params) {
#ifdef USE_MACROS
  uint16_t val;
  const char * macro;

  if (params != NULL) {
    if (notInRange(params, 0, 9, val)) return;
    //    execMacro((uint8_t)val);
    runMacro = (uint8_t)val;
  } else {
    for (int i = 0; i < 10; i++) {
      macro = (pgm_read_word(macros + i));
      //      dataStream.print(i);dataStream.print(F(": "));
      if (strlen_P(macro) > 0) {
        dataStream.print(i);
        dataStream.print(" ");
      }
    }
    dataStream.println();
  }
#else
  memset(params, '\0', 5);
  dataStream.println(F("Disabled"));
#endif
}


/***** Bus diagnostics *****/
/*
 * Usage: xdiag mode byte
 * mode: 0=data bus; 1=control bus
 * byte: byte to write on the bus
 * Note: values to switch individual bits = 1,2,4,8,10,20,40,80
 */
void xdiag_h(char *params){
  char *param;
  uint8_t mode = 0;
  uint8_t val = 0;
  
  // Get first parameter (mode = 0 or 1)
  param = strtok(params, " \t");
  if (param != NULL) {
    if (strlen(param)<4){
      mode = atoi(param);
      if (mode>2) {
        dataStream.println(F("Invalid: 0=data bus; 1=control bus"));
        return;
      }
    }
  }
  // Get second parameter (8 bit byte)
  param = strtok(NULL, " \t");
  if (param != NULL) {
    if (strlen(param)<4){
      val = atoi(param);
    }

    switch (mode) {
      case 0:
          // Set to required value
          gpibBus.setDataVal(val);
          // Reset after 10 seconds
          delay(10000);
          gpibBus.setDataVal(0);
          break;
      case 1:
          // Set to required state
          gpibBus.setControlVal(0xFF, 0xFF, 1);  // Set direction
          gpibBus.setControlVal(~val, 0xFF, 0);  // Set state (low=asserted so must be inverse of value)
          // Reset after 10 seconds
          delay(10000);
          if (gpibBus.cfg.cmode==2) {
            gpibBus.setControls(CINI);
          }else{
            gpibBus.setControls(DINI);
          }
          break;
#ifdef AR488_MCP23S17
      case 2:
        // MCP23S17 direction
        mcpByteWrite(MCPDIRA, 0b00000000);  // Port direction: 0 = output; 1 = input;
        mcpByteWrite(MCPPORTA, val);
        break;
#endif
    }
/*
    if (mode) {   // Control bus

    }else{        // Data bus

    }
*/
  }

}


/***** Enable Xon/Xoff handshaking for data transmission *****/
/*
void xonxoff_h(char *params){
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    xonxoff = val ? true : false;
    if (isVerb) {
      dataStream.print(F("Xon/Xoff: "));
      dataStream.println(val ? "ON" : "OFF") ;
    }
  } else {
    dataStream.println(xonxoff);
  }
}
*/


/****** Timing parameters ******/
/*
void tmbus_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 30000, val)) return;
    AR488.tmbus = val;
    if (isVerb) {
      dataStream.print(F("TmBus set to: "));
      dataStream.println(val);
    };
  } else {
    dataStream.println(AR488.tmbus, DEC);
  }
}
*/

/***** Set device ID *****/
/*
 * Sets the device ID parameters including:
 * ++id verstr - version string (same as ++setvstr)
 * ++id name   - short name of device (e.g. HP3478A) up to 15 characters
 * ++id serial - serial number up to 9 digits long
 */
void id_h(char *params) {
  uint8_t dlen = 0;
  char * keyword; // Pointer to keyword following ++id
  char * datastr; // Pointer to supplied data (remaining characters in buffer)
  char serialStr[10];

#ifdef DEBUG_IDFUNC
  dataStream.print(F("Params: "));
  dataStream.println(params);
#endif

  if (params != NULL) {
    keyword = strtok(params, " \t");
    datastr = keyword + strlen(keyword) + 1;
    dlen = strlen(datastr);
    if (dlen) {
      if (strncmp(keyword, "verstr", 6)==0) {
#ifdef DEBUG_IDFUNC
        dataStream.print(F("Keyword: "));
        dataStream.println(keyword);
        dataStream.print(F("DataStr: "));
        dataStream.println(datastr);
#endif
        if (dlen>0 && dlen<48) {
#ifdef DEBUG_IDFUNC
        dataStream.println(F("Length OK"));
#endif
          memset(gpibBus.cfg.vstr, '\0', 48);
          strncpy(gpibBus.cfg.vstr, datastr, dlen);
          if (isVerb) dataStream.print(F("VerStr: "));dataStream.println(gpibBus.cfg.vstr);
        }else{
          if (isVerb) dataStream.println(F("Length of version string must not exceed 48 characters!"));
          errBadCmd();
        }
        return;
      }
      if (strncasecmp(keyword, "name", 4)==0) {
        if (dlen>0 && dlen<16) {
          memset(gpibBus.cfg.sname, '\0', 16);
          strncpy(gpibBus.cfg.sname, datastr, dlen);
        }else{
          if (isVerb) dataStream.println(F("Length of name must not exceed 15 characters!"));
          errBadCmd();
        }
        return;
      }
      if (strncasecmp(keyword, "serial", 6)==0) {
        if (dlen < 10) {
          gpibBus.cfg.serial = atol(datastr);
        }else{
          if (isVerb) dataStream.println(F("Serial number must not exceed 9 characters!"));
          errBadCmd();
        }
        return;
      }
//      errBadCmd();
    }else{
      if (strncasecmp(keyword, "verstr", 6)==0) {
        dataStream.println(gpibBus.cfg.vstr);
        return;
      }
      if (strncasecmp(keyword, "fwver", 6)==0) {
        dataStream.println(F(FWVER));
        return;
      }
      if (strncasecmp(keyword, "name", 4)==0) {
        dataStream.println(gpibBus.cfg.sname);
        return;      
      } void addr_h(char *params);
      if (strncasecmp(keyword, "serial", 6)==0) {
        memset(serialStr, '\0', 10);
        snprintf(serialStr, 10, "%09lu", gpibBus.cfg.serial);  // Max str length = 10-1 i.e 9 digits + null terminator 
        dataStream.println(serialStr);
        return;    
      }
    }
  }
  errBadCmd();
#ifdef DEBUG_IDFUNC
    debugStream.println(F(id_h: done.));
#endif
}


void idn_h(char * params){
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 2, val)) return;
    gpibBus.cfg.idn = (uint8_t)val;
    if (isVerb) {
      dataStream.print(F("Sending IDN: "));
      dataStream.print(val ? "Enabled" : "Disabled"); 
      if (val==2) dataStream.print(F(" with serial number"));
      dataStream.println();
    };
  } else {
    dataStream.println(gpibBus.cfg.idn, DEC);
  }  
}


/***** Send device clear (usually resets the device to power on state) *****/
void sendmla_h() {
  if (gpibBus.sendMLA())  {
    if (isVerb) dataStream.println(F("Failed to send MLA"));
    return;
  }
}


/***** Send device clear (usually resets the device to power on state) *****/
void sendmta_h() {
  if (gpibBus.sendMTA())  {
    if (isVerb) dataStream.println(F("Failed to send MTA"));
    return;
  }
}


/***** Show or set read timout *****/
void sendmsa_h(char *params) {
/*
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 96, 126, val)) return;
    if (gpibBus.sendMSA(val)){
      if (isVerb) dataStream.println(F("Failed to send MSA"));
      return;
    }
  }
*/

  uint16_t saddr;
  char * param;
  if (params != NULL) {
    // Secondary address
    param = strtok(params, " \t");
    if (strlen(param) > 0) {
      if (notInRange(param, 96, 126, saddr)) return;
      if (gpibBus.sendMSA(saddr)){
        if (isVerb) dataStream.println(F("Failed to send MSA"));
        return;
      }
    }
    // Secondary address command parameter
    param = strtok(NULL, " \t");
    if (strlen(param)>0) {
      gpibBus.setControls(CTAS);
      gpibBus.sendData(param, strlen(param));
      gpibBus.setControls(CLAS);
    }
    addressingSuppressed = true;
  }
}


/***** Send device clear (usually resets the device to power on state) *****/
void unlisten_h() {
  if (gpibBus.sendUNL())  {
    if (isVerb) dataStream.println(F("Failed to send UNL"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
  addressingSuppressed = false;
}


/***** Send device clear (usually resets the device to power on state) *****/
void untalk_h() {
  if (gpibBus.sendUNT())  {
    if (isVerb) dataStream.println(F("Failed to send UNT"));
    return;
  }
  // Set GPIB controls back to idle state
  gpibBus.setControls(CIDS);
  addressingSuppressed = false;
}





/******************************************************/
/***** Device mode GPIB command handling routines *****/
/******************************************************/

/***** Attention handling routine *****/
/*
 * In device mode is invoked whenever ATN is asserted
 */
void attnRequired() {

  uint8_t db = 0;
  uint8_t stat = 0;
  bool mla = false;
  bool mta = false;
//  bool spe = false;
//  bool spd = false;
  bool eoiDetected = false;
  uint8_t gpibcmd = 0;
  uint8_t saddrcmd = 0;

//dataStream.print(F("attnRequired: LnRdy: "));
//dataStream.println(lnRdy);

  
  // Set device listner active state (assert NDAC+NRFD (low), DAV=INPUT_PULLUP)
  gpibBus.setControls(DLAS);

#ifdef DEBUG_DEVICE_ATN
  debugStream.println(F("attnRequired: Answering attention!"));
#endif

  // Read bytes
//  while (isATN) {
//  while (digitalRead(ATN)==LOW) {
  while (gpibBus.isAsserted(ATN)) {

#ifdef DEBUG_DEVICE_ATN
      debugStream.println(F("attnRequired: Reading byte..."));
#endif

    // Read the next byte from the bus, no EOI detection
    stat = gpibBus.readByte(&db, false, &eoiDetected);
    if (!stat) {

#ifdef DEBUG_DEVICE_ATN
      debugStream.println(db, HEX);
#endif

      // Device is addressed to listen
      if (gpibBus.cfg.paddr == (db ^ 0x20)) { // MLA = db^0x20
#ifdef DEBUG_DEVICE_ATN
        debugStream.println(F("attnRequired: Controller wants me to data accept data <<<"));
#endif
        mla = true;
//      }

      // Device is addressed to talk
      }else if (gpibBus.cfg.paddr == (db ^ 0x40)) { // MLA = db^0x40
        // Call talk handler to send data
        mta = true;
#ifdef DEBUG_DEVICE_ATN
        if (db != GC_SPE) debugStream.println(F("attnRequired: Controller wants me to send data >>>"));
#endif
      }else if (db>0x5F && db<0x80) {
        // Secondary addressing command received
        if (mla || mta) {
          saddrcmd = db;
#ifdef DEBUG_DEVICE_ATN
          debugStream.print(F("attnRequired: Secondary addressing command received: "));
          debugStream.println(saddrcmd, HEX);
#endif
        }
      }else{
        if (mla || mta) {
          gpibcmd = db;
#ifdef DEBUG_DEVICE_ATN
          debugStream.print(F("attnRequired: GPIB command received: "));
          debugStream.println(gpibcmd, HEX);
#endif
        }
      }
    }
  }



      // If we have been addressed then check if we need to take action
/*
      if (mla || mta) {


      // Serial poll enable request
      if (db==GC_SPE) spe = true;

      // Serial poll disable request
      if (db==GC_SPD) spd = true;
 
      // Unlisten
      if (db==GC_UNL) device_unl_h();

      // Untalk
      if (db==GC_UNT) device_unt_h();

      // Reset

    #ifdef PIN_REMOTE
      // Remote mode
      if (db==GC_LLO) device_llo_h();
      // Local mode
      if (db==GC_GTL) device_gtl_h();
    #endif


    }

  
  }
*/

#ifdef DEBUG_DEVICE_ATN
  debugStream.println(F("End ATN loop."));
#endif

/***** Perform GPIB copmmand actions *****/

  if (gpibcmd) {
    // Respond to GPIB command
    switch (db) {
      case GC_SPE:
        // Serial Poll enable request
#ifdef DEBUG_DEVICE_ATN
          debugStream.println(F("attnRequired: Received serial poll enable."));
#endif
          device_spe_h();
          break;
      case GC_SPD:
          // Serial poll disable request
#ifdef DEBUG_DEVICE_ATN
          debugStream.println(F("attnRequired: Received serial poll disable."));
#endif
          device_spd_h();
          mta = false;
          break;
      case GC_UNL:
          // Unlisten
          device_unl_h();
          break;
      case GC_UNT:
          // Untalk
          device_unt_h();
          break;
      case GC_SDC:
          // Device clear (reset)
          device_sdc_h();
          break;
#ifdef PIN_REMOTE
      case GC_LLO:
          // Remote lockout mode
          device_llo_h();
          break;
      case GC_GTL:
          // Local mode
          device_gtl_h();
          break;
#endif
    } // End switch
    // Clear flags
    mta = false;
    mla = false;
    gpibcmd = 0;
  }


/***** Perform secondry address command actions *****/

  if (saddrcmd) {

    // Clear flags
    saddrcmd = 0;
  }


/***** Otherwise perform read or write *****/
  // Listen for data
  if (mla) { 
#ifdef DEBUG_DEVICE_ATN
    debugStream.println(F("Listening..."));
#endif
    device_mla_h();
    mla = false;
  }

  // Talk (send data)
  if (mta) {
#ifdef DEBUG_DEVICE_ATN
    debugStream.println(F("Talking..."));
#endif
    device_mta_h();
    mta = false;
  }

  // Finished attention - set controls to idle
  gpibBus.setControls(DIDS);

#ifdef DEBUG_DEVICE_ATN
  debugStream.println(F("attnRequired: END attnReceived."));
#endif

}


/***** Device is addressed to listen - so listen *****/
void device_mla_h(){
  gpibBus.receiveData(dataStream, false, false, 0);
}


/***** Device is addressed to talk - so send data *****/
void device_mta_h(){
//  dataStream.print(F("mta_h: LnRdy: "));
//  dataStream.println(lnRdy);
  if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
}


/***** Selected Device Clear *****/
void device_sdc_h() {
  // If being addressed then reset
  if (isVerb) dataStream.println(F("Resetting..."));
#ifdef DEBUG_DEVICE_ATN
  debugStream.print(F("Reset adressed to me: "));
//  debugStream.println(aTl);
#endif
  rst_h();
  if (isVerb) dataStream.println(F("Reset failed."));
}


/***** Serial Poll Disable *****/
void device_spd_h() {
  if (isVerb) dataStream.println(F("<- serial poll request ended."));
}


/***** Serial Poll Enable *****/
void device_spe_h() {
  if (isVerb) dataStream.println(F("Serial poll request received from controller ->"));
  gpibBus.sendStatus();
  if (isVerb) dataStream.println(F("Status sent."));
  // Check if SRQ bit is set and clear it
  if (gpibBus.cfg.stat & 0x40) {
    gpibBus.setStatus(gpibBus.cfg.stat & ~0x40);
    if (isVerb) dataStream.println(F("SRQ bit cleared."));
  }
}


/***** Unlisten *****/
void device_unl_h() {
  // Stop receiving and go to idle
#ifdef DEBUG_DEVICE_ATN
  debugStream.println(F("Unlisten received."));
#endif
  readWithEoi = false;

  // Immediate break - shouldn't ATN do this anyway?
  tranBrk = 3;  // Stop receving transmission
  // Immediate break - shouldn't ATN do this anyway?
}


/***** Untalk *****/
void device_unt_h() {
  // Stop sending data and go to idle
#ifdef DEBUG_DEVICE_ATN
  debugStream.println(F("Untalk received."));
#endif
}


#ifdef REMOTE_SIGNAL_PIN
/***** Enabled remote mode *****/
void device_llo_h(){
  digitalWrite(PIN_REMOTE, HIGH);
}


/***** Disable remote mode *****/
void device_gtl_h(){
  digitalWrite(PIN_REMOTE, LOW);
}
#endif


void lonMode(){

  gpibBus.receiveData(dataStream, false, false, 0);

  // Clear the buffer to prevent it getting blocked
  if (lnRdy==2) flushPbuf();
}
