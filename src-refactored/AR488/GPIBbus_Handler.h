#ifndef GPIBBUS_HANDLER_H
#define GPIBBUS_HANDLER_H

//#include <SD.h>
#include "AR488_Config.h"
#include "AR488_ComPorts.h"


/*=============================================================*\
||                                                             ||
||       AR488 GPIB Interface,  ver. 0.55.21, 21/06/2026       ||
||   Twilight Logic, https://github.com/Twilight-Logic/AR488   ||
||                                                             ||
||                      GPIB BUS HANDLER                       ||
||                                                             ||
\*=============================================================*/


/*********************************************/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

#define GPIB_CFG_SIZE 85


/***** Universal Multiline commands (apply to all devices) *****/
#define GC_GTL 0x01
#define GC_SDC 0x04
#define GC_PPC 0x05
#define GC_GET 0x08
#define GC_TCT 0x09
#define GC_LLO 0x11
#define GC_DCL 0x14
#define GC_PPU 0x15
#define GC_SPE 0x18
#define GC_SPD 0x19
#define GC_LAD 0x20
#define GC_UNL 0x3F
#define GC_UNT 0x5F
#define GC_TAD 0x40
#define GC_PPE 0x60
#define GC_SAD 0x60
#define GC_PPD 0x70


/***** GPIB control states *****/
// Controller mode
#define CINI 0x01  // Controller idle state
#define CIDS 0x02  // Controller idle state
#define CCMS 0x03  // Controller command state
#define CTAS 0x04  // Controller talker active state
#define CLAS 0x05  // Controller listner active state
// Listner/device mode
#define DINI 0x06  // Device initialise state
#define DIDS 0x07  // Device idle state
#define DLAS 0x08  // Device listener active (listening/receiving)
#define DTAS 0x09  // Device talker active (sending) state


/***** BITS ASSIGNED TO GPIB BYTE *****/
#define IFC_BIT (1 << 0)
#define NDAC_BIT (1 << 1)
#define NRFD_BIT (1 << 2)
#define DAV_BIT (1 << 3)
#define EOI_BIT (1 << 4)
#define REN_BIT (1 << 5)
#define SRQ_BIT (1 << 6)
#define ATN_BIT (1 << 7)
#define CTRL_BITS (0xE1)
#define HSHK_BITS (0x1E)
#define ALL_BITS (0xFF)

/***** Addressing direction *****/
#define TONONE 0
#define TOLISTEN 1
#define TOTALK 2


/***** Lastbyte - send EOI *****/
#define NO_EOI false
#define WITH_EOI true


enum gpibHandshakeState: uint8_t {
  // Common
  HANDSHAKE_NOT_READY,
  HANDSHAKE_START,
  HANDSHAKE_COMPLETE,
  IFC_ASSERTED,
  ATN_ASSERTED,
  // Read
  WAIT_FOR_DATA,
  READ_DATA,
  DATA_ACCEPTED,
  // Write
  WAIT_FOR_RECEIVER_READY,
  PLACE_DATA,
  DATA_READY,
  RECEIVER_ACCEPTING
};


enum receiveState: uint8_t {
  RECEIVE_INIT,     // Initial state
  RECEIVE_BREAK,    // Receive interrupted by user command
  RECEIVE_ATN,      // Receive interrupted by ATN
  RECEIVE_IFC,      // Receive interrupted by IFC
  RECEIVE_EOI,      // Receive OK, terminated with EOI
  RECEIVE_ENDCHAR,  // Receive OK, terminated with custom end character
  RECEIVE_ENDL,     // Receive OK, terminated line of text (CR/LF)
  RECEIVE_LIMIT,    // Receive max byte count reached
  RECEIVE_ERR       // Receive timeout or error
};


enum operatingMode: uint8_t {
  OP_IDLE,
  OP_CTRL,
  OP_DEVI
};


enum transmitMode: uint8_t {
  TM_IDLE,
  TM_RECV,
  TM_SEND,
  NONE
};


/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/*********************************************/


/****************************************/
/***** GPIB CLASS OBJECT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

class GPIBbus {


public:

  /***** Controller configuration *****/
  union GPIBconf {
    struct {
      bool eot_en;      // Enable/disable append EOT char to string received from GPIB bus before sending to USB
      bool eoi;         // Assert EOI on last data char written to GPIB - 0-disable, 1-enable
      uint8_t cmode;    // Controller/device mode (0=unset, 1=device, 2=controller)
      uint8_t caddr;    // This interface address
      uint8_t paddr;    // Primary address to use when addressing a device
      uint8_t saddr;    // Secondary address to use when addressing a device
      uint8_t eos;      // EOS (end of send to GPIB) characters [0=CRLF, 1=CR, 2=LF, 3=None]
      uint8_t stat;     // Status byte to return in response to a serial poll
      uint8_t amode;    // Auto mode setting (0=off; 1=Prologix; 2=onquery; 3=continuous);
      uint16_t rtmo;    // Read timout (read_tmo_ms) in milliseconds - 0-32000 - value depends on instrument
      uint8_t eot_ch;   // EOT character to append to USB output when EOI signal detected (0-255)
      char vstr[48];    // Custom version string
      uint8_t eor;      // EOR (end of receive from GPIB instrument) characters [0=CRLF, 1=CR, 2=LF, 3=None, 4=LFCR, 5=ETX, 6=CRLF+ETX, 7=SPACE]
      char sname[16];   // Interface short name
      uint32_t serial;  // Serial number
      uint8_t idn;      // Send ID in response to *idn? 0=disable, 1=send name; 2=send name+serial
      uint8_t hflags;   // Handshaking indicator flags
      uint16_t aint;    // Auto-read interval in milliseconds when auto = 3
    };
    uint8_t db[GPIB_CFG_SIZE];
  };

  GPIBconf cfg;


  GPIBbus();

  void begin();
  void stop();

  void setDefaultCfg();

  void startControllerMode();
  void startDeviceMode();

  void setOperatingMode(enum operatingMode mode);
  void setTransmitMode(enum transmitMode mode);
  void assertSignal(uint8_t sig);
  void clearSignal(uint8_t sig);
  void clearAllSignals();
  void clearDataBus();


  bool isController();

  void sendIFC();
  bool sendLLO();
  bool sendGTL();
  bool sendGET(uint8_t addr);
  bool sendSDC();
  bool sendTCT(uint8_t addr);
  void sendAllClear();

  bool sendUNT();
  bool sendUNL();
  bool sendMTA();
  bool sendMLA();
  bool sendMSA(uint8_t addr);

  bool isAsserted(uint8_t gpibsig);
  void setControls(uint8_t state);
  void sendStatus();

  void setStatus(uint8_t statusByte);
  bool sendCmd(uint8_t cmdByte);
//  bool sendSecondaryCmd(uint8_t paddr, uint8_t saddr, char * data, uint8_t dsize);
  enum gpibHandshakeState readByte(uint8_t *db, bool readWithEoi, bool *eoi);
  enum gpibHandshakeState writeByte(uint8_t db, bool eoi);
  enum receiveState receiveData(Stream &dataStream, bool detectEoi, bool detectEndByte, uint8_t endByte, size_t maxSize = 0);
  void sendData(const char *data, size_t dsize, bool eoi);
//  void clearDataBus();
  void setControlVal(uint8_t value);
  void setDataVal(uint8_t value);

  bool isDeviceAddressedToListen();
  bool isDeviceAddressedToTalk();
  bool isDeviceInIdleState();

  void signalBreak();

  bool addressDevice(uint8_t pri, uint8_t sec, uint8_t dir);
  bool unAddressDevice();
  uint8_t haveAddressedDevice();

private:

  uint8_t cstate = 0;
  bool txBreak;  // Signal to break the GPIB transmission
  uint8_t deviceAddressed;
  bool isTerminatorDetected(uint8_t bytes[3], uint8_t eorSequence);
  enum transmitMode _xmitMode;

  // Interrupt flag for MCP23S17
#ifdef AR488_MCP23S17
//    extern volatile bool mcpIntA;  // MCP23S17 interrupt handler
//    uint8_t mcpPinAssertedReg = 0;
#endif
};


#endif  // GPIBBUS_HANDLER_H
