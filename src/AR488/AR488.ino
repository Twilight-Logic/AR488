//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wtype-limits"
//#pragma GCC diagnostic ignored "-Wunused-variable"

#ifdef __AVR__
  #include <avr/wdt.h>
#endif

//#pragma GCC diagnostic pop

#include "AR488_Config.h"
#include "AR488_Layouts.h"
#include "AR488_Eeprom.h"

#ifdef USE_INTERRUPTS
  #ifdef __AVR__
    #include <avr/interrupt.h>
  #endif
#endif
#ifdef E2END
  #include <EEPROM.h>
  
#endif

#ifdef AR_BT_EN
  #include "AR488_BT.h"
#endif


/***** FWVER "AR488 GPIB controller, ver. 0.49.12, 11/01/2021" *****/

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
   Standard commands

   ++addr         - display/set device address
   ++auto         - automatically request talk and read response
   ++clr          - send Selected Device Clear to current GPIB address
   ++eoi          - enable/disable assertion of EOI signal
   ++eos          - specify GPIB termination character
   ++eot_enable   - enable/disable appending user specified character to USB output on EOI detection
   ++eot_char     - set character to append to USB output when EOT enabled
   ++ifc          - assert IFC signal for 150 miscoseconds - make AR488 controller in charge
   ++llo          - local lockout - disable front panel operation on instrument
   ++loc          - enable front panel operation on instrument
   ++lon          - put controller in listen-only mode (listen to all traffic)
   ++mode         - set the interface mode (0=controller/1=device)
   ++read         - read data from instrument
   ++read_tmo_ms  - read timeout specified between 1 - 3000 milliseconds
   ++rst          - reset the controller
   ++savecfg      - save configration
   ++spoll        - serial poll the addressed host or all instruments
   ++srq          - return status of srq signal (1-srq asserted/0-srq not asserted)
   ++status       - set the status byte to be returned on being polled (bit 6 = RQS, i.e SRQ asserted)
   ++trg          - send trigger to selected devices (up to 15 addresses)
   ++ver          - display firmware version
*/

/*
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


/***** Serial/debug port *****/
#ifdef AR_CDC_SERIAL
  Serial_ *arSerial = &(AR_SERIAL_PORT);
  #ifndef DB_SERIAL_PORT
    Serial_ *dbSerial = arSerial;
  #endif
#endif
#ifdef AR_HW_SERIAL
  HardwareSerial *arSerial = &(AR_SERIAL_PORT);
  #ifndef DB_SERIAL_PORT
    HardwareSerial *dbSerial = arSerial;
  #endif
#endif
// Note: SoftwareSerial support conflicts with PCINT support
#ifdef AR_SW_SERIAL
  #include <SoftwareSerial.h>
  SoftwareSerial swArSerial(AR_SW_SERIAL_RX, AR_SW_SERIAL_TX);
  SoftwareSerial *arSerial = &swArSerial;
  #ifndef DB_SERIAL_PORT
    SoftwareSerial *dbSerial = arSerial;
  #endif
#endif


/***** Debug Port *****/
#ifdef DB_SERIAL_PORT
  #ifdef DB_CDC_SERIAL
    Serial_ *dbSerial = &(DB_SERIAL_PORT);
  #endif
  #ifdef DB_HW_SERIAL
    HardwareSerial *dbSerial = &(DB_SERIAL_PORT);
  #endif
  // Note: SoftwareSerial support conflicts with PCINT support
  #ifdef DB_SW_SERIAL
    #include <SoftwareSerial.h>
    SoftwareSerial swDbSerial(DB_SW_SERIAL_RX, DB_SW_SERIAL_TX);
    SoftwareSerial *dbSerial = &swDbSerial;
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
union AR488conf{
  struct{
//    uint8_t ew;       // EEPROM write indicator byte
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


/****** Global variables with volatile values related to controller state *****/

// GPIB control state
uint8_t cstate = 0;

// Verbose mode
bool isVerb = false;

// CR/LF terminated line ready to process
uint8_t lnRdy = 0;      

// GPIB data receive flags
//bool isReading = false; // Is a GPIB read in progress?
bool aRead = false;     // GPIB data read in progress
bool rEoi = false;      // Read eoi requested
bool rEbt = false;      // Read with specified terminator character
bool isQuery = false;   // Direct instrument command is a query
uint8_t tranBrk = 0;    // Transmission break on 1=++, 2=EOI, 3=ATN 4=UNL
uint8_t eByte = 0;      // Termination character

// Device mode - send data
bool snd = false;

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
bool deviceAddressing = true;   // Suppress sending commands to address the instrument
bool dataBufferFull = false;    // Flag when parse buffer is full

// State flags set by interrupt being triggered
extern volatile bool isATN;  // has ATN been asserted?
extern volatile bool isSRQ;  // has SRQ been asserted?

// SRQ auto mode
bool isSrqa = false;

// Interrupt without handler fired
//volatile bool isBAD = false;

// Whether to run Macro 0 (macros must be enabled)
uint8_t runMacro = 0;

// Send response to *idn?
bool sendIdn = false;

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

#ifdef USE_INTERRUPTS
  // Turn on interrupts on port
  interruptsEn();
#endif

  // Initialise parse buffer
  flushPbuf();

// Initialise debug port
#ifdef DB_SERIAL_PORT
  if (dbSerial != arSerial) dbSerial->begin(DB_SERIAL_BAUD);
#endif

 // Initialise serial comms over USB or Bluetooth
#ifdef AR_BT_EN
  // Initialise Bluetooth  
  btInit();
  arSerial->begin(AR_BT_BAUD);
#else
  // Start the serial port
  arSerial->begin(AR_SERIAL_BAUD);
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
  initAR488();

#ifdef E2END
  // Read data from non-volatile memory
  //(will only read if previous config has already been saved)
//  epGetCfg();
  if (!isEepromClear()) {
    if (!epReadData(AR488.db, AR_CFG_SIZE)) {
      // CRC check failed - config data does not match EEPROM
      epErase();
      initAR488();
      epWriteData(AR488.db, AR_CFG_SIZE);
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

  // Initialize the interface in device mode
  if (AR488.cmode == 1) initDevice();

  // Initialize the interface in controller mode
  if (AR488.cmode == 2) initController();

  isATN = false;
  isSRQ = false;

#if defined(USE_MACROS) && defined(RUN_STARTUP)
  // Run startup macro
  execMacro(0);
#endif

#ifdef SAY_HELLO
  arSerial->println(F("AR488 ready."));
#endif

}
/****** End of Arduino standard SETUP procedure *****/


/***** ARDUINO MAIN LOOP *****/
void loop() {

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

/*** Pin Hooks ***/
/*
 * Not all boards support interrupts or have PCINTs. In this
 * case, use in-loop checking to detect when SRQ and ATN have 
 * been signalled
 */
#ifndef USE_INTERRUPTS
  isATN = (digitalRead(ATN)==LOW ? true : false);
  isSRQ = (digitalRead(SRQ)==LOW ? true : false);
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

  // lnRdy=1: received a command so execute it...
  if (lnRdy == 1) {
    execCmd(pBuf, pbPtr);
  }

  // Controller mode:
  if (AR488.cmode == 2) {
    // lnRdy=2: received data - send it to the instrument...
    if (lnRdy == 2) {
      sendToInstrument(pBuf, pbPtr);
      // Auto-read data from GPIB bus following any command
      if (AR488.amode == 1) {
        //        delay(10);
        gpibReceiveData();
      }
      // Auto-receive data from GPIB bus following a query command
      if (AR488.amode == 2 && isQuery) {
        //        delay(10);
        gpibReceiveData();
        isQuery = false;
      }
    }

    // Check status of SRQ and SPOLL if asserted
    if (isSRQ && isSrqa) {
      spoll_h(NULL);
      isSRQ = false;
    }

    // Continuous auto-receive data from GPIB bus
    if (AR488.amode == 3 && aRead) gpibReceiveData();
  }

  // Device mode:
  if (AR488.cmode == 1) {
    if (isTO) {
      if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
    }else if (isRO) {
      lonMode();
    }else{
      if (isATN) attnRequired();
      if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
    }
  }

  // Reset line ready flag
//  lnRdy = 0;

  // IDN query ?
  if (sendIdn) {
    if (AR488.idn==1) arSerial->println(AR488.sname);
    if (AR488.idn==2) {arSerial->print(AR488.sname);arSerial->print("-");arSerial->println(AR488.serial);}
    sendIdn = false;
  }

  // Check serial buffer
  lnRdy = serialIn_h();
  
  delayMicroseconds(5);
}
/***** END MAIN LOOP *****/


/***** Initialise the interface *****/
void initAR488() {
  // Set default values ({'\0'} sets version string array to null)
  AR488 = {false, false, 2, 0, 1, 0, 0, 0, 0, 1200, 0, {'\0'}, 0, 0, {'\0'}, 0, 0};
}


/***** Initialise device mode *****/
void initDevice() {
  // Set GPIB control bus to device idle mode
  setGpibControls(DINI);

  // Initialise GPIB data lines (sets to INPUT_PULLUP)
  readGpibDbus();
}


/***** Initialise controller mode *****/
void initController() {
  // Set GPIB control bus to controller idle mode
  setGpibControls(CINI);  // Controller initialise state
  // Initialise GPIB data lines (sets to INPUT_PULLUP)
  readGpibDbus();
  // Assert IFC to signal controller in charge (CIC)
  ifc_h();
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
  while (arSerial->available() && bufferStatus==0) {   // Parse while characters available and line is not complete
    bufferStatus = parseInput(arSerial->read());
  }

#ifdef DEBUG1
  if (bufferStatus) {
    dbSerial->print(F("BufferStatus: "));
    dbSerial->println(bufferStatus);  
  }
#endif

  return bufferStatus;
}


/***** Detect ATN state *****/
/*
 * When interrupts are being used the state is automatically flagged when
 * the ATN interrupt is triggered. Where the interrupt cannot be used the
 * state of the ATN line needs to be checked.
 */
bool isAtnAsserted() {
#ifdef USE_INTERRUPTS
  if (isATN) return true;
#else
  // ATN is LOW when asserted
  if (digitalRead(ATN) == LOW) return true;
#endif
  return false;
}


/*************************************/
/***** Device operation routines *****/
/*************************************/


/***** Unrecognized command *****/
void errBadCmd() {
  arSerial->println(F("Unrecognized command"));
}


/***** Add character to the buffer and parse *****/
uint8_t parseInput(char c) {

  uint8_t r = 0;

  // Read until buffer full (buffer-size - 2 characters)
  if (pbPtr < PBSIZE) {
    if (isVerb) arSerial->print(c);  // Humans like to see what they are typing...
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
            if (isVerb) showPrompt();
            return 0;
          } else {
            if (isVerb) arSerial->println();  // Move to new line
#ifdef DEBUG1
            dbSerial->print(F("parseInput: Received ")); dbSerial->println(pBuf);
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
            }else if (pbPtr>3 && AR488.idn>0 && isIdnQuery(pBuf)){
              sendIdn = true;
              flushPbuf();
            // Buffer has at least 1 character = instrument data to send to gpib bus
            }else if (pbPtr > 0) {
              r = 2;
            }
            isPlusEscaped = false;
#ifdef DEBUG1
            dbSerial->print(F("R: "));dbSerial->println(r);
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
//        if (isVerb) arSerial->print(c);
        break;
      // Something else?
      default: // any char other than defined above
//        if (isVerb) arSerial->print(c);  // Humans like to see what they are typing...
        // Buffer contains '++' (start of command). Stop sending data to serial port by halting GPIB receive.
        addPbuf(c);
        isEsc = false;
    }
  }
  if (pbPtr >= PBSIZE) {
    if (isCmd(pBuf) && !r) {  // Command without terminator and buffer full
      if (isVerb) {
        arSerial->println(F("ERROR - Command buffer overflow!"));
      }
      flushPbuf();
    }else{  // Buffer contains data and is full, so process the buffer (send data via GPIB)
      dataBufferFull = true;
      r = 2;
    }
  }
  return r;
}


/***** Is this a command? *****/
bool isCmd(char *buffr) {
  if (buffr[0] == PLUS && buffr[1] == PLUS) {
#ifdef DEBUG1
    dbSerial->println(F("isCmd: Command detected."));
#endif
    return true;
  }
  return false;
}


/***** Is this an *idn? query? *****/
bool isIdnQuery(char *buffr) {
//  if (buffr[0] == PLUS && buffr[1] == PLUS) {
  if (strncmp(buffr, "*idn?", 5)==0) {
#ifdef DEBUG1
    dbSerial->println(F("isIdnQuery: Detected IDN query."));
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
  { "mode" ,       3, cmode_h     },
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
  { "ver",         3, ver_h       },
  { "verbose",     3, (void(*)(char*)) verb_h    },
  { "tmbus",       3, tmbus_h     },
  { "xdiag",       3, xdiag_h     }

};


/***** Show a prompt *****/
void showPrompt() {
  // Print prompt
  arSerial->println();
  arSerial->print("> ");
}


/****** Send data to instrument *****/
/* Processes the parse buffer whenever a full CR or LF
 * and sends data to the instrument
 */
void sendToInstrument(char *buffr, uint8_t dsize) {

#ifdef DEBUG1
  dbSerial->print(F("sendToInstrument: Received for sending: ")); printHex(buffr, dsize);
#endif

  // Is this an instrument query command (string ending with ?)
  if (buffr[dsize-1] == '?') isQuery = true;

  // Send string to instrument
  gpibSendData(buffr, dsize);
  // Clear data buffer full flag
  if (dataBufferFull) dataBufferFull = false;

  // Show a prompt on completion?
  if (isVerb) showPrompt();

  // Flush the parse buffer
  flushPbuf();
  lnRdy = 0;

#ifdef DEBUG1
  dbSerial->println(F("sendToInstrument: Sent."));
#endif

}


/***** Execute a command *****/
void execCmd(char *buffr, uint8_t dsize) {
  char line[PBSIZE];

  // Copy collected chars to line buffer
  memcpy(line, buffr, dsize);

  // Flush the parse buffer
  flushPbuf();
  lnRdy = 0;

#ifdef DEBUG1
  dbSerial->print(F("execCmd: Command received: ")); printHex(line, dsize);
#endif

  // Its a ++command so shift everything two bytes left (ignore ++) and parse
  for (int i = 0; i < dsize-2; i++) {
    line[i] = line[i + 2];
  }
  // Replace last two bytes with a null (\0) character
  line[dsize - 2] = '\0';
  line[dsize - 1] = '\0';
#ifdef DEBUG1
  dbSerial->print(F("execCmd: Sent to the command processor: ")); printHex(line, dsize-2);
#endif
  // Execute the command
  if (isVerb) arSerial->println(); // Shift output to next line
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

#ifdef DEBUG1
  dbSerial->print("getCmd: ");
  dbSerial->print(buffr); dbSerial->print(F(" - length:")); dbSerial->println(strlen(buffr));
#endif

  // If terminator on blank line then return immediately without processing anything 
  if (buffr[0] == 0x00) return;
  if (buffr[0] == CR) return;
  if (buffr[0] == LF) return;

  // Get the first token
  token = strtok(buffr, " \t");

#ifdef DEBUG1
  dbSerial->print("getCmd: process token: "); dbSerial->println(token);
#endif

  // Check whether it is a valid command token
  i = 0;
  do {
    if (strcasecmp(cmdHidx[i].token, token) == 0) break;
    i++;
  } while (i < casize);

  if (i < casize) {
    // We have found a valid command and handler
#ifdef DEBUG1
    dbSerial->print("getCmd: found handler for: "); dbSerial->println(cmdHidx[i].token);
#endif
    // If command is relevant to controller mode then execute it
    if (cmdHidx[i].opmode & AR488.cmode) {
      // If its a command with parameters
      // Copy command parameters to params and call handler with parameters
      params = token + strlen(token) + 1;
  
      // If command parameters were specified
      if (strlen(params) > 0) {
#ifdef DEBUG1
        dbSerial->print(F("Calling handler with parameters: ")); dbSerial->println(params);
#endif
        // Call handler with parameters specified
        cmdHidx[i].handler(params);
      }else{
        // Call handler without parameters
        cmdHidx[i].handler(NULL);
      }   
    }else{
      errBadCmd();
      if (isVerb) arSerial->println(F("Command not available in this mode."));
    }

  } else {
    // No valid command found
    errBadCmd();
  }
 
}


/***** Prints charaters as hex bytes *****/
void printHex(char *buffr, int dsize) {
  for (int i = 0; i < dsize; i++) {
    dbSerial->print(buffr[i], HEX); dbSerial->print(" ");
  }
  dbSerial->println();
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
      arSerial->print(F("Valid range is between "));
      arSerial->print(lowl);
      arSerial->print(F(" and "));
      arSerial->println(higl);
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
      // Reached last character before NL. Add to buffer before processing
      if (i == ssize-1) {
        // Check buffer and add character
        if (pbPtr < (PBSIZE - 2)){
          addPbuf(c);
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
      if (pbPtr < (PBSIZE - 2)) {
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
  char *param;
  uint16_t val;
  if (params != NULL) {

    // Primary address
    param = strtok(params, " \t");
    if (notInRange(param, 1, 30, val)) return;
    if (val == AR488.caddr) {
      errBadCmd();
      if (isVerb) arSerial->println(F("That is my address! Address of a remote device is required."));
      return;
    }
    AR488.paddr = val;
    if (isVerb) {
      arSerial->print(F("Set device primary address to: "));
      arSerial->println(val);
    }

    // Secondary address
    AR488.saddr = 0;
    val = 0;
    param = strtok(NULL, " \t");
    if (param != NULL) {
      if (notInRange(param, 96, 126, val)) return;
      AR488.saddr = val;
      if (isVerb) {
        arSerial->print("Set device secondary address to: ");
        arSerial->println(val);
      }
    }

  } else {
    arSerial->print(AR488.paddr);
    if (AR488.saddr > 0) {
      arSerial->print(F(" "));
      arSerial->print(AR488.saddr);
    }
    arSerial->println();
  }
}


/***** Show or set read timout *****/
void rtmo_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 1, 32000, val)) return;
    AR488.rtmo = val;
    if (isVerb) {
      arSerial->print(F("Set [read_tmo_ms] to: "));
      arSerial->print(val);
      arSerial->println(F(" milliseconds"));
    }
  } else {
    arSerial->println(AR488.rtmo);
  }
}


/***** Show or set end of send character *****/
void eos_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 3, val)) return;
    AR488.eos = (uint8_t)val;
    if (isVerb) {
      arSerial->print(F("Set EOS to: "));
      arSerial->println(val);
    };
  } else {
    arSerial->println(AR488.eos);
  }
}


/***** Show or set EOI assertion on/off *****/
void eoi_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    AR488.eoi = val ? true : false;
    if (isVerb) {
      arSerial->print(F("Set EOI assertion: "));
      arSerial->println(val ? "ON" : "OFF");
    };
  } else {
    arSerial->println(AR488.eoi);
  }
}


/***** Show or set interface to controller/device mode *****/
void cmode_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    switch (val) {
      case 0:
        AR488.cmode = 1;
        initDevice();
        break;
      case 1:
        AR488.cmode = 2;
        initController();
        break;
    }
    if (isVerb) {
      arSerial->print(F("Interface mode set to: "));
      arSerial->println(val ? "CONTROLLER" : "DEVICE");
    }
  } else {
    arSerial->println(AR488.cmode - 1);
  }
}


/***** Show or enable/disable sending of end of transmission character *****/
void eot_en_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    AR488.eot_en = val ? true : false;
    if (isVerb) {
      arSerial->print(F("Appending of EOT character: "));
      arSerial->println(val ? "ON" : "OFF");
    }
  } else {
    arSerial->println(AR488.eot_en);
  }
}


/***** Show or set end of transmission character *****/
void eot_char_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 255, val)) return;
    AR488.eot_ch = (uint8_t)val;
    if (isVerb) {
      arSerial->print(F("EOT set to ASCII character: "));
      arSerial->println(val);
    };
  } else {
    arSerial->println(AR488.eot_ch, DEC);
  }
}


/***** Show or enable/disable auto mode *****/
void amode_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 3, val)) return;
    if (val > 0 && isVerb) {
      arSerial->println(F("WARNING: automode ON can cause some devices to generate"));
      arSerial->println(F("         'addressed to talk but nothing to say' errors"));
    }
    AR488.amode = (uint8_t)val;
    if (AR488.amode < 3) aRead = false;
    if (isVerb) {
      arSerial->print(F("Auto mode: "));
      arSerial->println(AR488.amode);
    }
  } else {
    arSerial->println(AR488.amode);
  }
}


/***** Display the controller version string *****/
void ver_h(char *params) {
  // If "real" requested
  if (params != NULL && strncmp(params, "real", 3) == 0) {
    arSerial->println(F(FWVER));
    // Otherwise depends on whether we have a custom string set
  } else {
    if (strlen(AR488.vstr) > 0) {
      arSerial->println(AR488.vstr);
    } else {
      arSerial->println(F(FWVER));
    }
  }
}


/***** Address device to talk and read the sent data *****/
void read_h(char *params) {
  // Clear read flags
  rEoi = false;
  rEbt = false;
  // Read any parameters
  if (params != NULL) {
    if (strlen(params) > 3) {
      if (isVerb) arSerial->println(F("Invalid termination character - ignored!")); void addr_h(char *params);
    } else if (strncmp(params, "eoi", 3) == 0) { // Read with eoi detection
      rEoi = true;
    } else { // Assume ASCII character given and convert to an 8 bit byte
      rEbt = true;
      eByte = atoi(params);
    }
  }
  if (AR488.amode == 3) {
    // In auto continuous mode we set this flag to indicate we are ready for continuous read
    aRead = true;
  } else {
    // If auto mode is disabled we do a single read
    gpibReceiveData();
  }
}


/***** Send device clear (usually resets the device to power on state) *****/
void clr_h() {
  if (addrDev(AR488.paddr, 0)) {
    if (isVerb) arSerial->println(F("Failed to address device"));
    return;
  }
  if (gpibSendCmd(GC_SDC))  {
    if (isVerb) arSerial->println(F("Failed to send SDC"));
    return;
  }
  if (uaddrDev()) {
    if (isVerb) arSerial->println(F("Failed to untalk GPIB bus"));
    return;
  }
  // Set GPIB controls back to idle state
  setGpibControls(CIDS);
}


/***** Send local lockout command *****/
void llo_h(char *params) {
  // NOTE: REN *MUST* be asserted (LOW)
  if (digitalRead(REN)==LOW) {
    // For 'all' send LLO to the bus without addressing any device - devices will show REM
    if (params != NULL) {
      if (0 == strncmp(params, "all", 3)) {
        if (gpibSendCmd(GC_LLO)) {
          if (isVerb) arSerial->println(F("Failed to send universal LLO."));
        }
      }
    } else {
      // Address current device
      if (addrDev(AR488.paddr, 0)) {
        if (isVerb) arSerial->println(F("Failed to address the device."));
        return;
      }
      // Send LLO to currently addressed device
      if (gpibSendCmd(GC_LLO)) {
        if (isVerb) arSerial->println(F("Failed to send LLO to device"));
        return;
      }
      // Unlisten bus
      if (uaddrDev()) {
        if (isVerb) arSerial->println(F("Failed to unlisten the GPIB bus"));
        return;
      }
    }
  }
  // Set GPIB controls back to idle state
  setGpibControls(CIDS);
}


/***** Send Go To Local (GTL) command *****/
void loc_h(char *params) {
  // REN *MUST* be asserted (LOW)
  if (digitalRead(REN)==LOW) {
    if (params != NULL) {
      if (strncmp(params, "all", 3) == 0) {
        // Un-assert REN
        setGpibState(0b00100000, 0b00100000, 0);
        delay(40);
        // Simultaneously assert ATN and REN
        setGpibState(0b00000000, 0b10100000, 0);
        delay(40);
        // Unassert ATN
        setGpibState(0b10000000, 0b10000000, 0);
      }
    } else {
      // Address device to listen
      if (addrDev(AR488.paddr, 0)) {
        if (isVerb) arSerial->println(F("Failed to address device."));
        return;
      }
      // Send GTL
      if (gpibSendCmd(GC_GTL)) {
        if (isVerb) arSerial->println(F("Failed sending LOC."));
        return;
      }
      // Unlisten bus
      if (uaddrDev()) {
        if (isVerb) arSerial->println(F("Failed to unlisten GPIB bus."));
        return;
      }
      // Set GPIB controls back to idle state
      setGpibControls(CIDS);
    }
  }
}


/***** Assert IFC for 150 microseconds *****/
/* This indicates that the AR488 the Controller-in-Charge on
 * the bus and causes all interfaces to return to their idle 
 * state
 */
void ifc_h() {
  if (AR488.cmode) {
    // Assert IFC
    setGpibState(0b00000000, 0b00000001, 0);
    delayMicroseconds(150);
    // De-assert IFC
    setGpibState(0b00000001, 0b00000001, 0);
    if (isVerb) arSerial->println(F("IFC signal asserted for 150 microseconds"));
  }
}


/***** Send a trigger command *****/
void trg_h(char *params) {
  char *param;
  uint8_t addrs[15];
  uint16_t val = 0;
  uint8_t cnt = 0;

  // Initialise address array
  for (int i = 0; i < 15; i++) {
    addrs[i] = 0;
  }

  // Read parameters
  if (params == NULL) {
    // No parameters - trigger addressed device only
    addrs[0] = AR488.paddr;
    cnt++;
  } else {
    // Read address parameters into array
    while (cnt < 15) {
      if (cnt == 0) {
        param = strtok(params, " \t");
      } else {
        param = strtok(NULL, " \t");
      }
      if (notInRange(param, 1, 30, val)) return;
      addrs[cnt] = (uint8_t)val;
      cnt++;
    }
  }

  // If we have some addresses to trigger....
  if (cnt > 0) {
    for (int i = 0; i < cnt; i++) {
      // Address the device
      if (addrDev(addrs[i], 0)) {
        if (isVerb) arSerial->println(F("Failed to address device"));
        return;
      }
      // Send GTL
      if (gpibSendCmd(GC_GET))  {
        if (isVerb) arSerial->println(F("Failed to trigger device"));
        return;
      }
      // Unaddress device
      if (uaddrDev()) {
        if (isVerb) arSerial->println(F("Failed to unlisten GPIB bus"));
        return;
      }
    }

    // Set GPIB controls back to idle state
    setGpibControls(CIDS);

    if (isVerb) arSerial->println(F("Group trigger completed."));
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
    arSerial->println(F("Reset FAILED."));
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
    addrs[0] = AR488.paddr;
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
        if (isVerb) arSerial->println(F("Serial poll of all devices requested..."));
        break;
        // Read all address parameters
      } else if (strlen(params) < 3) { // No more than 2 characters
        if (notInRange(param, 1, 30, val)) return;
        addrs[j] = (uint8_t)val;
        j++;
      } else {
        errBadCmd();
        if (isVerb) arSerial->println(F("Invalid parameter"));
        return;
      }
    }
  }

  // Send Unlisten [UNL] to all devices
  if ( gpibSendCmd(GC_UNL) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send UNL"));
#endif
    return;
  }

  // Controller addresses itself as listner
  if ( gpibSendCmd(GC_LAD + AR488.caddr) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send LAD"));
#endif
    return;
  }

  // Send Serial Poll Enable [SPE] to all devices
  if ( gpibSendCmd(GC_SPE) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send SPE"));
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
    if (val != AR488.caddr) {

      // Address a device to talk
      if ( gpibSendCmd(GC_TAD + val) )  {

#ifdef DEBUG4
        dbSerial->println(F("spoll_h: failed to send TAD"));
#endif
        return;
      }

      // Set GPIB control to controller active listner state (ATN unasserted)
      setGpibControls(CLAS);

      // Read the response byte (usually device status) using handshake
      r = gpibReadByte(&sb, &eoiDetected);

      // If we successfully read a byte
      if (!r) {
        if (j > 1) {
          // If all, return specially formatted response: SRQ:addr,status
          // but only when RQS bit set
          if (sb & 0x40) {
            arSerial->print(F("SRQ:")); arSerial->print(i); arSerial->print(F(",")); arSerial->println(sb, DEC);
            i = j;
          }
        } else {
          // Return decimal number representing status byte
          arSerial->println(sb, DEC);
          if (isVerb) {
            arSerial->print(F("Received status byte ["));
            arSerial->print(sb);
            arSerial->print(F("] from device at address: "));
            arSerial->println(val);
          }
          i = j;
        }
      } else {
        if (isVerb) arSerial->println(F("Failed to retrieve status byte"));
      }
    }
  }
  if (all) arSerial->println();

  // Send Serial Poll Disable [SPD] to all devices
  if ( gpibSendCmd(GC_SPD) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send SPD"));
#endif
    return;
  }

  // Send Untalk [UNT] to all devices
  if ( gpibSendCmd(GC_UNT) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send UNT"));
#endif
    return;
  }

  // Unadress listners [UNL] to all devices
  if ( gpibSendCmd(GC_UNL) )  {
#ifdef DEBUG4
    dbSerial->println(F("spoll_h: failed to send UNL"));
#endif
    return;
  }

  // Set GPIB control to controller idle state
  setGpibControls(CIDS);

  // Set SRQ to status of SRQ line. Should now be unasserted but, if it is
  // still asserted, then another device may be requesting service so another
  // serial poll will be called from the main loop
  if (digitalRead(SRQ) == LOW) {
    isSRQ = true;
  } else {
    isSRQ = false;
  }
  if (isVerb) arSerial->println(F("Serial poll completed."));

}


/***** Return status of SRQ line *****/
void srq_h() {
  //NOTE: LOW=asserted, HIGH=unasserted
  arSerial->println(!digitalRead(SRQ));
}


/***** Set the status byte (device mode) *****/
void stat_h(char *params) {
  uint16_t val = 0;
  // A parameter given?
  if (params != NULL) {
    // Byte value given?
    if (notInRange(params, 0, 255, val)) return;
    AR488.stat = (uint8_t)val;
    if (val & 0x40) {
      setSrqSig();
      if (isVerb) arSerial->println(F("SRQ asserted."));
    } else {
      clrSrqSig();
      if (isVerb) arSerial->println(F("SRQ un-asserted."));
    }
  } else {
    // Return the currently set status byte
    arSerial->println(AR488.stat);
  }
}


/***** Save controller configuration *****/
void save_h() {
#ifdef E2END
  epWriteData(AR488.db, AR_CFG_SIZE);
  if (isVerb) arSerial->println(F("Settings saved."));
#else
  arSerial->println(F("EEPROM not supported."));
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
      arSerial->print(F("LON: "));
      arSerial->println(val ? "ON" : "OFF") ;
    }
  } else {
    arSerial->println(isRO);
  }
}


/***** Set the SRQ signal *****/
void setSrqSig() {
  // Set SRQ line to OUTPUT HIGH (asserted)
  setGpibState(0b01000000, 0b01000000, 1);
  setGpibState(0b00000000, 0b01000000, 0);
}


/***** Clear the SRQ signal *****/
void clrSrqSig() {
  // Set SRQ line to INPUT_PULLUP (un-asserted)
  setGpibState(0b00000000, 0b01000000, 1);
  setGpibState(0b01000000, 0b01000000, 0);
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
  if ( gpibSendCmd(GC_DCL) )  {
    if (isVerb) arSerial->println(F("Sending DCL failed"));
    return;
  }
  // Set GPIB controls back to idle state
  setGpibControls(CIDS);
}


/***** Re-load default configuration *****/
void default_h() {
  initAR488();
}


/***** Show or set end of receive character(s) *****/
void eor_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 15, val)) return;
    AR488.eor = (uint8_t)val;
    if (isVerb) {
      arSerial->print(F("Set EOR to: "));
      arSerial->println(val);
    };
  } else {
    if (AR488.eor>7) AR488.eor = 0;  // Needed to reset FF read from EEPROM after FW upgrade
    arSerial->println(AR488.eor);
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
  setGpibControls(CIDS);
  delayMicroseconds(20);
  // Assert ATN and EOI
  setGpibState(0b00000000, 0b10010000, 0);
  //  setGpibState(0b10010000, 0b00000000, 0b10010000);
  delayMicroseconds(20);
  // Read data byte from GPIB bus without handshake
  sb = readGpibDbus();
  // Return to controller idle state (ATN and EOI unasserted)
  setGpibControls(CIDS);

  // Output the response byte
  arSerial->println(sb, DEC);

  if (isVerb) arSerial->println(F("Parallel poll completed."));
}


/***** Assert or de-assert REN 0=de-assert; 1=assert *****/
void ren_h(char *params) {
#if defined (SN7516X) && not defined (SN7516X_DC)
  params = params;
  arSerial->println(F("Unavailable")) ;
#else
  // char *stat;
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 1, val)) return;
    digitalWrite(REN, (val ? LOW : HIGH));
    if (isVerb) {
      arSerial->print(F("REN: "));
      arSerial->println(val ? "REN asserted" : "REN un-asserted") ;
    };
  } else {
    arSerial->println(digitalRead(REN) ? 0 : 1);
  }
#endif
}


/***** Enable verbose mode 0=OFF; 1=ON *****/
void verb_h() {
  isVerb = !isVerb;
  arSerial->print("Verbose: ");
  arSerial->println(isVerb ? "ON" : "OFF");
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
  strncat(idparams, params, plen);

/*
arSerial->print(F("Plen: "));
arSerial->println(plen);
arSerial->print(F("Params: "));
arSerial->println(params);
arSerial->print(F("IdParams: "));
arSerial->println(idparams);
*/

  id_h(idparams);
  
/*  
  if (params != NULL) {
    len = strlen(params);
    if (len>47) len=47; // Ignore anything over 47 characters
    memset(AR488.vstr, '\0', 48);
    strncpy(AR488.vstr, params, len);
    if (isVerb) {
      arSerial->print(F("Changed version string to: "));
      arSerial->println(params);
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
      arSerial->print(F("TON: "));
      arSerial->println(val ? "ON" : "OFF") ;
    }
  } else {
    arSerial->println(isTO);
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
    if (isVerb) arSerial->println(isSrqa ? "SRQ auto ON" : "SRQ auto OFF") ;
  } else {
    arSerial->println(isSrqa);
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
        gpibSendData(param, strlen(param));
        delay(tmdly);
        gpibReceiveData();
      }
    } else {
      errBadCmd();
      if (isVerb) arSerial->println(F("Missing parameter"));
      return;
    }
  } else {
    errBadCmd();
    if (isVerb) arSerial->println(F("Missing parameters"));
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
      //      arSerial->print(i);arSerial->print(F(": "));
      if (strlen_P(macro) > 0) {
        arSerial->print(i);
        arSerial->print(" ");
      }
    }
    arSerial->println();
  }
#else
  memset(params, '\0', 5);
  arSerial->println(F("Disabled"));
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
        arSerial->println(F("Invalid: 0=data bus; 1=control bus"));
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

    if (mode) {   // Control bus
      // Set to required state
      setGpibState(0xFF, 0xFF, 1);  // Set direction
      setGpibState(~val, 0xFF, 0);  // Set state (low=asserted so must be inverse of value)
      // Reset after 10 seconds
      delay(10000);
      if (AR488.cmode==2) {
        setGpibControls(CINI);
      }else{
        setGpibControls(DINI);
      }
    }else{        // Data bus
      // Set to required value
      setGpibDbus(val);
      // Reset after 10 seconds
      delay(10000);
      setGpibDbus(0);
    }
  }

}


/****** Timing parameters ******/

void tmbus_h(char *params) {
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 30000, val)) return;
    AR488.tmbus = val;
    if (isVerb) {
      arSerial->print(F("TmBus set to: "));
      arSerial->println(val);
    };
  } else {
    arSerial->println(AR488.tmbus, DEC);
  }
}


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

#ifdef DEBUG10
  arSerial->print(F("Params: "));
  arSerial->println(params);
#endif

  if (params != NULL) {
    keyword = strtok(params, " \t");
    datastr = keyword + strlen(keyword) + 1;
    dlen = strlen(datastr);
    if (dlen) {
      if (strncmp(keyword, "verstr", 6)==0) {
#ifdef DEBUG10       
        arSerial->print(F("Keyword: "));
        arSerial->println(keyword);
        arSerial->print(F("DataStr: "));
        arSerial->println(datastr);
#endif
        if (dlen>0 && dlen<48) {
#ifdef DEBUG10
        arSerial->println(F("Length OK"));
#endif
          memset(AR488.vstr, '\0', 48);
          strncpy(AR488.vstr, datastr, dlen);
          if (isVerb) arSerial->print(F("VerStr: "));arSerial->println(AR488.vstr);
        }else{
          if (isVerb) arSerial->println(F("Length of version string must not exceed 48 characters!"));
          errBadCmd();
        }
        return;
      }
      if (strncmp(keyword, "name", 4)==0) {
        if (dlen>0 && dlen<16) {
          memset(AR488.sname, '\0', 16);
          strncpy(AR488.sname, datastr, dlen);
        }else{
          if (isVerb) arSerial->println(F("Length of name must not exceed 15 characters!"));
          errBadCmd();
        }
        return;
      }
      if (strncmp(keyword, "serial", 6)==0) {
        if (dlen < 10) {
          AR488.serial = atol(datastr);
        }else{
          if (isVerb) arSerial->println(F("Serial number must not exceed 9 characters!"));
          errBadCmd();
        }
        return;
      }
//      errBadCmd();
    }else{
      if (strncmp(keyword, "verstr", 6)==0) {
        arSerial->println(AR488.vstr);
        return;
      }     
      if (strncmp(keyword, "name", 4)==0) {
        arSerial->println(AR488.sname);
        return;      
      }
      if (strncmp(keyword, "serial", 6)==0) {
        memset(serialStr, '\0', 10);
        snprintf(serialStr, 10, "%09lu", AR488.serial);  // Max str length = 10-1 i.e 9 digits + null terminator 
        arSerial->println(serialStr);
        return;    
      }
    }
  }
  errBadCmd();
}


void idn_h(char * params){
  uint16_t val;
  if (params != NULL) {
    if (notInRange(params, 0, 2, val)) return;
    AR488.idn = (uint8_t)val;
    if (isVerb) {
      arSerial->print(F("Sending IDN: "));
      arSerial->print(val ? "Enabled" : "Disabled"); 
      if (val==2) arSerial->print(F(" with serial number"));
      arSerial->println();
    };
  } else {
    arSerial->println(AR488.idn, DEC);
  }  
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
  bool spe = false;
  bool spd = false;
  bool eoiDetected = false;
  
  // Set device listner active state (assert NDAC+NRFD (low), DAV=INPUT_PULLUP)
  setGpibControls(DLAS);

#ifdef DEBUG5
  dbSerial->println(F("Answering attention!"));
#endif

  // Read bytes
//  while (isATN) {
  while (digitalRead(ATN)==LOW) {
    stat = gpibReadByte(&db, &eoiDetected);
    if (!stat) {

#ifdef DEBUG5
      dbSerial->println(db, HEX);
#endif

      // Device is addressed to listen
      if (AR488.paddr == (db ^ 0x20)) { // MLA = db^0x20
#ifdef DEBUG5
        dbSerial->println(F("attnRequired: Controller wants me to data accept data <<<"));
#endif
        mla = true;
      }

      // Device is addressed to talk
      if (AR488.paddr == (db ^ 0x40)) { // MLA = db^0x40
          // Call talk handler to send data
          mta = true;
#ifdef DEBUG5
          if (!spe) dbSerial->println(F("attnRequired: Controller wants me to send data >>>"));
#endif
      }

      // Serial poll enable request
      if (db==GC_SPE) spe = true;

      // Serial poll disable request
      if (db==GC_SPD) spd = true;
 
      // Unlisten
      if (db==GC_UNL) unl_h();

      // Untalk
      if (db==GC_UNT) unt_h();

    }
  
  }

#ifdef DEBUG5
  dbSerial->println(F("End ATN loop."));
#endif

  if (mla) { 
#ifdef DEBUG5
    dbSerial->println(F("Listening..."));
#endif
    // Call listen handler (receive data)
    mla_h();
    mla = false;
  }

  // Addressed to listen?
  if (mta) {
    // Serial poll enabled
    if (spe) {
#ifdef DEBUG5
      dbSerial->println(F("attnRequired: Received serial poll enable."));
#endif
      spe_h();
      spe = false;
    // Otherwise just send data
    }else{
      mta_h();
      mta = false;
    }
  }

  // Serial poll disable received
  if (spd) {
#ifdef DEBUG5
    dbSerial->println(F("attnRequired: Received serial poll disable."));
#endif
    spd_h();
    mta = false;
    spe = false;
    spd = false;
  }

  // Finished attention - set controls to idle
  setGpibControls(DIDS);

#ifdef DEBUG5
  dbSerial->println(F("attnRequired: END attnReceived."));
#endif

}


/***** Device is addressed to listen - so listen *****/
void mla_h(){
  gpibReceiveData();
}


/***** Device is addressed to talk - so send data *****/
void mta_h(){
  if (lnRdy == 2) sendToInstrument(pBuf, pbPtr);
}


/***** Selected Device Clear *****/
void sdc_h() {
  // If being addressed then reset
  if (isVerb) arSerial->println(F("Resetting..."));
#ifdef DEBUG5
  dbSerial->print(F("Reset adressed to me: ")); dbSerial->println(aTl);
#endif
  if (aTl) rst_h();
  if (isVerb) arSerial->println(F("Reset failed."));
}


/***** Serial Poll Disable *****/
void spd_h() {
  if (isVerb) arSerial->println(F("<- serial poll request ended."));
}


/***** Serial Poll Enable *****/
void spe_h() {
  if (isVerb) arSerial->println(F("Serial poll request received from controller ->"));
  gpibSendStatus();
  if (isVerb) arSerial->println(F("Status sent."));
  // Clear the SRQ bit
  AR488.stat = AR488.stat & ~0x40;
  // Clear the SRQ signal
  clrSrqSig();
  if (isVerb) arSerial->println(F("SRQ bit cleared (if set)."));
}


/***** Unlisten *****/
void unl_h() {
  // Stop receiving and go to idle
#ifdef DEBUG5
  dbSerial->println(F("Unlisten received."));
#endif
  rEoi = false;
  tranBrk = 3;  // Stop receving transmission
}


/***** Untalk *****/
void unt_h() {
  // Stop sending data and go to idle
#ifdef DEBUG5
  dbSerial->println(F("Untalk received."));
#endif
}


void lonMode(){

  gpibReceiveData();

  // Clear the buffer to prevent it getting blocked
  if (lnRdy==2) flushPbuf();
  
}


/***************************************/
/***** GPIB DATA HANDLING ROUTINES *****/
/***************************************/

/*****  Send a single byte GPIB command *****/
bool gpibSendCmd(uint8_t cmdByte) {

  bool stat = false;

  // Set lines for command and assert ATN
  setGpibControls(CCMS);

  // Send the command
  stat = gpibWriteByte(cmdByte);
  if (stat && isVerb) {
    arSerial->print(F("gpibSendCmd: failed to send command "));
    arSerial->print(cmdByte, HEX);
    arSerial->println(F(" to device"));
  }

  // Return to controller idle state
  //  setGpibControls(CIDS);
  // NOTE: this breaks serial poll

  return stat ? ERR : OK;
}


/***** Send the status byte *****/
void gpibSendStatus() {
  // Have been addressed and polled so send the status byte
  if (isVerb) {
    arSerial->print(F("Sending status byte: "));
    arSerial->println(AR488.stat);
  };
  setGpibControls(DTAS);
  gpibWriteByte(AR488.stat);
  setGpibControls(DIDS);
}


/***** Send a series of characters as data to the GPIB bus *****/
void gpibSendData(char *data, uint8_t dsize) {

  bool err = false;

  // If lon is turned on we cannot send data so exit
  if (isRO) return;

  // Controler can unlisten bus and address devices
  if (AR488.cmode == 2) {

    if (deviceAddressing) {
      // Address device to listen
      if (addrDev(AR488.paddr, 0)) {
        if (isVerb) {
          arSerial->print(F("gpibSendData: failed to address device "));
          arSerial->print(AR488.paddr);
          arSerial->println(F(" to listen"));
        }
        return;
      }
    }

    deviceAddressing = dataBufferFull ? false : true;

#ifdef DEBUG3
    dbSerial->println(F("Device addressed."));
#endif

    // Set control lines to write data (ATN unasserted)
    setGpibControls(CTAS);

  } else {
    setGpibControls(DTAS);
  }
#ifdef DEBUG3
  dbSerial->println(F("Set write data mode."));
  dbSerial->print(F("Send->"));
#endif

  // Write the data string
  for (int i = 0; i < dsize; i++) {
    // If EOI asserting is on
    if (AR488.eoi) {
      // Send all characters
      err = gpibWriteByte(data[i]);
    } else {
      // Otherwise ignore non-escaped CR, LF and ESC
      if ((data[i] != CR) || (data[i] != LF) || (data[i] != ESC)) err = gpibWriteByte(data[i]);
    }
#ifdef DEBUG3
    dbSerial->print(data[i]);
#endif
    if (err) break;
  }

#ifdef DEBUG3
  dbSerial->println("<-End.");
#endif

  if (!err) {
    // Write terminators according to EOS setting
    // Do we need to write a CR?
    if ((AR488.eos & 0x2) == 0) {
      gpibWriteByte(CR);
#ifdef DEBUG3
      dbSerial->println(F("Appended CR"));
#endif
    }
    // Do we need to write an LF?
    if ((AR488.eos & 0x1) == 0) {
      gpibWriteByte(LF);
#ifdef DEBUG3
      dbSerial->println(F("Appended LF"));
#endif
    }
  }

  // If EOI enabled and no more data to follow then assert EOI
  if (AR488.eoi && !dataBufferFull) {
    setGpibState(0b00000000, 0b00010000, 0);
    //    setGpibState(0b00010000, 0b00000000, 0b00010000);
    delayMicroseconds(40);
    setGpibState(0b00010000, 0b00010000, 0);
    //    setGpibState(0b00010000, 0b00010000, 0b00010000);
#ifdef DEBUG3
    dbSerial->println(F("Asserted EOI"));
#endif
  }

  if (AR488.cmode == 2) {   // Controller mode
    if (!err) {
      if (deviceAddressing) {
        // Untalk controller and unlisten bus
        if (uaddrDev()) {
          if (isVerb) arSerial->println(F("gpibSendData: Failed to unlisten bus"));
        }

#ifdef DEBUG3
        dbSerial->println(F("Unlisten done"));
#endif
      }
    }

    // Controller - set lines to idle?
    setGpibControls(CIDS);

  }else{    // Device mode
    // Set control lines to idle
    if (AR488.cmode == 1) setGpibControls(DIDS);
  }

#ifdef DEBUG3
    dbSerial->println(F("<- End of send."));
#endif
 
}


/***** Receive data from the GPIB bus ****/
/*
 * Readbreak:
 * 5 - EOI detected
 * 7 - command received via serial
 */
bool gpibReceiveData() {

  uint8_t r = 0; //, db;
  uint8_t bytes[3] = {0};
  uint8_t eor = AR488.eor&7;
  int x = 0;
  bool eoiStatus;
  bool eoiDetected = false;

  // Reset transmission break flag
  tranBrk = 0;

  // Set status of EOI detection
  eoiStatus = rEoi; // Save status of rEoi flag
  if (AR488.eor==7) rEoi = true;    // Using EOI as terminator

  // Set up for reading in Controller mode
  if (AR488.cmode == 2) {   // Controler mode
    // Address device to talk
    if (addrDev(AR488.paddr, 1)) {
      if (isVerb) {
        arSerial->print(F("Failed to address the device"));
        arSerial->print(AR488.paddr);
        arSerial->println(F(" to talk"));
      }
    }
    // Wait for instrument ready
    Wait_on_pin_state(HIGH, NRFD, AR488.rtmo);
    // Set GPIB control lines to controller read mode
    setGpibControls(CLAS);
    
  // Set up for reading in Device mode
  } else {  // Device mode
    // Set GPIB controls to device read mode
    setGpibControls(DLAS);
    rEoi = true;  // In device mode we read with EOI by default
  }

#ifdef DEBUG7
    dbSerial->println(F("gpibReceiveData: Start listen ->"));
    dbSerial->println(F("Before loop flags:"));
    dbSerial->print(F("TRNb: "));
    dbSerial->println(tranBrk);
    dbSerial->print(F("rEOI: "));
    dbSerial->println(rEoi);
    dbSerial->print(F("ATN:  "));
    dbSerial->println(isAtnAsserted() ? 1 : 0);
#endif

  // Ready the data bus
  readyGpibDbus();

  // Perform read of data (r=0: data read OK; r>0: GPIB read error);
  while (r == 0) {

    // Tranbreak > 0 indicates break condition
    if (tranBrk > 0) break;

    // ATN asserted
    if (isAtnAsserted()) break;

    // Read the next character on the GPIB bus
    r = gpibReadByte(&bytes[0], &eoiDetected);

    // When reading with amode=3 or EOI check serial input and break loop if neccessary
    if ((AR488.amode==3) || rEoi) lnRdy = serialIn_h();
    
    // Line terminator detected (loop breaks on command being detected or data buffer full)
    if (lnRdy > 0) {
      aRead = false;  // Stop auto read
      break;
    }

#ifdef DEBUG7
    if (eoiDetected) dbSerial->println(F("\r\nEOI detected."));
#endif

    // If break condition ocurred or ATN asserted then break here
    if (isAtnAsserted()) break;

#ifdef DEBUG7
    dbSerial->print(bytes[0], HEX), dbSerial->print(' ');
#else
    // Output the character to the serial port
    arSerial->print((char)bytes[0]);
#endif

    // Byte counter
    x++;

    // EOI detection enabled and EOI detected?
    if (rEoi) {
      if (eoiDetected) break;
    }else{
      // Has a termination sequence been found ?
      if (isTerminatorDetected(bytes, eor)) break;
    }

    // Stop on timeout
    if (r > 0) break;

    // Shift last three bytes in memory
    bytes[2] = bytes[1];
    bytes[1] = bytes[0];
  }

#ifdef DEBUG7
  dbSerial->println();
  dbSerial->println(F("After loop flags:"));
  dbSerial->print(F("ATN: "));
  dbSerial->println(isAtnAsserted());
  dbSerial->print(F("TMO:  "));
  dbSerial->println(r);
#endif

  // End of data - if verbose, report how many bytes read
  if (isVerb) {
    arSerial->print(F("Bytes read: "));
    arSerial->println(x);
  }

  // Detected that EOI has been asserted
  if (eoiDetected) {
    if (isVerb) arSerial->println(F("EOI detected!"));
    // If eot_enabled then add EOT character
    if (AR488.eot_en) arSerial->print(AR488.eot_ch);
  }

  // Return rEoi to previous state
  rEoi = eoiStatus;

  // Verbose timeout error
  if (r > 0) {
    if (isVerb && r == 1) arSerial->println(F("Timeout waiting for sender!"));
    if (isVerb && r == 2) arSerial->println(F("Timeout waiting for transfer to complete!"));
  }

  // Return controller to idle state
  if (AR488.cmode == 2) {

    // Untalk bus and unlisten controller
    if (uaddrDev()) {
      if (isVerb) arSerial->print(F("gpibSendData: Failed to untalk bus"));
    }

    // Set controller back to idle state
    if (AR488.cmode == 2) setGpibControls(CIDS);

  } else {
    // Set device back to idle state
    setGpibControls(DIDS);
  }

#ifdef DEBUG7
    dbSerial->println(F("<- End listen."));
#endif

  // Reset flags
//  isReading = false;
  if (tranBrk > 0) tranBrk = 0;

  if (r > 0) return ERR;

  return OK;
}


/***** Check for terminator *****/
bool isTerminatorDetected(uint8_t bytes[3], uint8_t eor_sequence){
  if (rEbt) {
    // Stop on specified <char> if appended to ++read command
    if (bytes[0] == eByte) return true;
  }else{
    // Look for specified terminator (CR+LF by default)
    switch (eor_sequence) {
      case 0:
          // CR+LF terminator
          if (bytes[0]==LF && bytes[1]==CR) return true;
          break;
      case 1:
          // CR only as terminator
          if (bytes[0]==CR) return true;
          break;
      case 2:
          // LF only as terminator
          if (bytes[0]==LF) return true;
          break;
      case 3:
          // No terminator (will rely on timeout)
          break;
      case 4:
          // Keithley can use LF+CR instead of CR+LF
          if (bytes[0]==CR && bytes[1]==LF) return true;
          break;
      case 5:
          // Solarton (possibly others) can also use ETX (0x03)
          if (bytes[0]==0x03) return true;
          break;
      case 6:
          // Solarton (possibly others) can also use CR+LF+ETX (0x03)
          if (bytes[0]==0x03 && bytes[1]==LF && bytes[2]==CR) return true;
          break;
      default:
          // Use CR+LF terminator by default
          if (bytes[0]==LF && bytes[1]==CR) return true;
          break;
      }
  }
  return false;
}


/***** Read a SINGLE BYTE of data from the GPIB bus using 3-way handshake *****/
/*
 * (- this function is called in a loop to read data    )
 * (- the GPIB bus must already be configured to listen )
 */
uint8_t gpibReadByte(uint8_t *db, bool *eoi) {
  bool atnStat = (digitalRead(ATN) ? false : true); // Set to reverse, i.e. asserted=true; unasserted=false;
  *eoi = false;

  // Unassert NRFD (we are ready for more data)
  setGpibState(0b00000100, 0b00000100, 0);

  // ATN asserted and just got unasserted - abort - we are not ready yet
  if (atnStat && (digitalRead(ATN)==HIGH)) {
    setGpibState(0b00000000, 0b00000100, 0);
    return 3;
  }

  // Wait for DAV to go LOW indicating talker has finished setting data lines..
  if (Wait_on_pin_state(LOW, DAV, AR488.rtmo))  {
    if (isVerb) arSerial->println(F("gpibReadByte: timeout waiting for DAV to go LOW"));
    setGpibState(0b00000000, 0b00000100, 0);
    // No more data for you?
    return 1;
  }

  // Assert NRFD (NOT ready - busy reading data)
  setGpibState(0b00000000, 0b00000100, 0);

  // Check for EOI signal
  if (rEoi && digitalRead(EOI) == LOW) *eoi = true;

  // read from DIO
  *db = readGpibDbus();

  // Unassert NDAC signalling data accepted
  setGpibState(0b00000010, 0b00000010, 0);

  // Wait for DAV to go HIGH indicating data no longer valid (i.e. transfer complete)
  if (Wait_on_pin_state(HIGH, DAV, AR488.rtmo))  {
    if (isVerb) arSerial->println(F("gpibReadByte: timeout waiting DAV to go HIGH"));
    return 2;
  }

  // Re-assert NDAC - handshake complete, ready to accept data again
  setGpibState(0b00000000, 0b00000010, 0);

  // GPIB bus DELAY
  delayMicroseconds(AR488.tmbus);

  return 0;

}


/***** Write a SINGLE BYTE onto the GPIB bus using 3-way handshake *****/
/*
 * (- this function is called in a loop to send data )
 */
bool gpibWriteByte(uint8_t db) {

  bool err;

  err = gpibWriteByteHandshake(db);

  // Unassert DAV
  setGpibState(0b00001000, 0b00001000, 0);

  // Reset the data bus
  setGpibDbus(0);

  // GPIB bus DELAY
  delayMicroseconds(AR488.tmbus);

  // Exit successfully
  return err;
}


/***** GPIB send byte handshake *****/
bool gpibWriteByteHandshake(uint8_t db) {
  
    // Wait for NDAC to go LOW (indicating that devices are at attention)
  if (Wait_on_pin_state(LOW, NDAC, AR488.rtmo)) {
    if (isVerb) arSerial->println(F("gpibWriteByte: timeout waiting for receiver attention [NDAC asserted]"));
    return true;
  }
  // Wait for NRFD to go HIGH (indicating that receiver is ready)
  if (Wait_on_pin_state(HIGH, NRFD, AR488.rtmo))  {
    if (isVerb) arSerial->println(F("gpibWriteByte: timeout waiting for receiver ready - [NRFD unasserted]"));
    return true;
  }

  // Place data on the bus
  setGpibDbus(db);

  // Assert DAV (data is valid - ready to collect)
  setGpibState(0b00000000, 0b00001000, 0);

  // Wait for NRFD to go LOW (receiver accepting data)
  if (Wait_on_pin_state(LOW, NRFD, AR488.rtmo))  {
    if (isVerb) arSerial->println(F("gpibWriteByte: timeout waiting for data to be accepted - [NRFD asserted]"));
    return true;
  }

  // Wait for NDAC to go HIGH (data accepted)
  if (Wait_on_pin_state(HIGH, NDAC, AR488.rtmo))  {
    if (isVerb) arSerial->println(F("gpibWriteByte: timeout waiting for data accepted signal - [NDAC unasserted]"));
    return true;
  }

  return false;
}


/***** Untalk bus then address a device *****/
/*
 * dir: 0=listen; 1=talk;
 */
bool addrDev(uint8_t addr, bool dir) {
  if (gpibSendCmd(GC_UNL)) return ERR;
  if (dir) {
    // Device to talk, controller to listen
    if (gpibSendCmd(GC_TAD + addr)) return ERR;
    if (gpibSendCmd(GC_LAD + AR488.caddr)) return ERR;
  } else {
    // Device to listen, controller to talk
    if (gpibSendCmd(GC_LAD + addr)) return ERR;
    if (gpibSendCmd(GC_TAD + AR488.caddr)) return ERR;
  }
  return OK;
}


/***** Unaddress a device (untalk bus) *****/
bool uaddrDev() {
  // De-bounce
  delayMicroseconds(30);
  // Utalk/unlisten
  if (gpibSendCmd(GC_UNL)) return ERR;
  if (gpibSendCmd(GC_UNT)) return ERR;
  return OK;
}


/**********************************/
/*****  GPIB CONTROL ROUTINES *****/
/**********************************/


/***** Wait for "pin" to reach a specific state *****/
/*
 * Returns false on success, true on timeout.
 * Pin MUST be set as INPUT_PULLUP otherwise it will not change and simply time out!
 */
boolean Wait_on_pin_state(uint8_t state, uint8_t pin, int interval) {

  unsigned long timeout = millis() + interval;
  bool atnStat = (digitalRead(ATN) ? false : true); // Set to reverse - asserted=true; unasserted=false;

  while (digitalRead(pin) != state) {
    // Check timer
    if (millis() >= timeout) return true;
    // ATN status was asserted but now unasserted so abort
    if (atnStat && (digitalRead(ATN)==HIGH)) return true;
    //    if (digitalRead(EOI)==LOW) tranBrk = 2;
  }
  return false;        // = no timeout therefore succeeded!
}

/***** Control the GPIB bus - set various GPIB states *****/
/*
 * state is a predefined state (CINI, CIDS, CCMS, CLAS, CTAS, DINI, DIDS, DLAS, DTAS);
 * Bits control lines as follows: 8-ATN, 7-SRQ, 6-REN, 5-EOI, 4-DAV, 3-NRFD, 2-NDAC, 1-IFC
 * setGpibState byte1 (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
 * setGpibState byte2 (mask)     : 0=unaffected, 1=enabled
 * setGpibState byte3 (mode)     : 0=set pin state, 1=set pin direction
 */
void setGpibControls(uint8_t state) {

  // Switch state
  switch (state) {
    // Controller states

    case CINI:  // Initialisation
      // Set pin direction
      setGpibState(0b10111000, 0b11111111, 1);
      // Set pin state
      setGpibState(0b11011111, 0b11111111, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
  #ifdef SN7516X_DC
        digitalWrite(SN7516X_DC,LOW);
  #endif
  #ifdef SN7516X_SC
        digitalWrite(SN7516X_SC,HIGH);
  #endif
#endif      
#ifdef DEBUG2
      dbSerial->println(F("Initialised GPIB control mode"));
#endif
      break;

    case CIDS:  // Controller idle state
      setGpibState(0b10111000, 0b10011110, 1);
      setGpibState(0b11011111, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines to idle state"));
#endif
      break;

    case CCMS:  // Controller active - send commands
      setGpibState(0b10111001, 0b10011111, 1);
      setGpibState(0b01011111, 0b10011111, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines for sending a command"));
#endif
      break;

    case CLAS:  // Controller - read data bus
      // Set state for receiving data
      setGpibState(0b10100110, 0b10011110, 1);
      setGpibState(0b11011000, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines for reading data"));
#endif
      break;

    case CTAS:  // Controller - write data bus
      setGpibState(0b10111001, 0b10011110, 1);
      setGpibState(0b11011111, 0b10011110, 0);
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines for writing data"));
#endif
      break;

    /* Bits control lines as follows: 8-ATN, 7-SRQ, 6-REN, 5-EOI, 4-DAV, 3-NRFD, 2-NDAC, 1-IFC */

    // Listener states
    case DINI:  // Listner initialisation
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
  #ifdef SN7516X_DC
        digitalWrite(SN7516X_DC,HIGH);
  #endif
  #ifdef SN7516X_SC
        digitalWrite(SN7516X_SC,LOW);
  #endif
#endif      
      setGpibState(0b00000000, 0b11111111, 1);
      setGpibState(0b11111111, 0b11111111, 0);
#ifdef DEBUG2
      dbSerial->println(F("Initialised GPIB listener mode"));
#endif
      break;

    case DIDS:  // Device idle state
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
      setGpibState(0b00000000, 0b00001110, 1);
      setGpibState(0b11111111, 0b00001110, 0);
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines to idle state"));
#endif
      break;

    case DLAS:  // Device listner active (actively listening - can handshake)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,LOW);
#endif      
      setGpibState(0b00000110, 0b00001110, 1);
      setGpibState(0b11111001, 0b00001110, 0);
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines to idle state"));
#endif
      break;

    case DTAS:  // Device talker active (sending data)
#ifdef SN7516X
      digitalWrite(SN7516X_TE,HIGH);
#endif      
      setGpibState(0b00001000, 0b00001110, 1);
      setGpibState(0b11111001, 0b00001110, 0);
#ifdef DEBUG2
      dbSerial->println(F("Set GPIB lines for listening as addresed device"));
#endif
      break;
#ifdef DEBUG2

    default:
      // Should never get here!
      //      setGpibState(0b00000110, 0b10111001, 0b11111111);
      dbSerial->println(F("Unknown GPIB state requested!"));
#endif
  }

  // Save state
  cstate = state;

  // GPIB bus delay (to allow state to settle)
  delayMicroseconds(AR488.tmbus);

}
