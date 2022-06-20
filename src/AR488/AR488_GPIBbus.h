#ifndef AR488_GPIBbus_H
#define AR488_GPIBbus_H

//#include <SD.h>
#include "AR488_Config.h"
#include "AR488_Layouts.h"
#include "AR488_ComPorts.h"

#ifdef EN_STORAGE
  #ifdef EN_TEK_4924
    #include "AR488_Store_Tek_4924.h"
  #endif
#endif

/***** AR488_GPIBbus.cpp, ver. 0.51.08, 20/06/2022 *****/


/*********************************************/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

#define GPIB_CFG_SIZE 83

/***** Debug Port *****/
#ifdef DB_SERIAL_ENABLE
  extern Stream& debugStream;
  /* Configured in Config.h */
  //#define DEBUG_GPIBbus_RECEIVE
  //#define DEBUG_GPIBbus_SEND
  //#define DEBUG_GPIBbus_CONTROL
  //#define SN7516X
  /* Configured in Config.h */

#endif


/***** Universal Multiline commands (apply to all devices) *****/
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


/***** Addressing direction *****/
#define TALK true
#define LISTEN false

/***** Lastbyte - send EOI *****/
#define NO_EOI false
#define WITH_EOI true

/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** GPIB COMMAND & STATUS DEFINITIONS *****/
/*********************************************/


/****************************************/
/***** GPIB CLASS OBJECT DEFINITION *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

class GPIBbus {


  public:

    /***** Controller configuration *****/
    union GPIBconf{
      struct{
        bool eot_en;      // Enable/disable append EOT char to string received from GPIB bus before sending to USB
        bool eoi;         // Assert EOI on last data char written to GPIB - 0-disable, 1-enable
        uint8_t cmode;    // Controller/device mode (0=unset, 1=device, 2=controller)
        uint8_t caddr;    // This interface address
        uint8_t paddr;    // Primary address to use when addressing device
        uint8_t saddr;    // Secondary address to use when addressing device
        uint8_t eos;      // EOS (end of send to GPIB) characters [0=CRLF, 1=CR, 2=LF, 3=None]
        uint8_t stat;     // Status byte to return in response to a serial poll
        uint8_t amode;    // Auto mode setting (0=off; 1=Prologix; 2=onquery; 3=continuous);
        int rtmo;         // Read timout (read_tmo_ms) in milliseconds - 0-3000 - value depends on instrument
        char eot_ch;      // EOT character to append to USB output when EOI signal detected
        char vstr[48];    // Custom version string
//        uint16_t tmbus;   // Delay to allow the bus control/data lines to settle (1-30,000 microseconds)
        uint8_t eor;      // EOR (end of receive from GPIB instrument) characters [0=CRLF, 1=CR, 2=LF, 3=None, 4=LFCR, 5=ETX, 6=CRLF+ETX, 7=SPACE]
        char sname[16];   // Interface short name
        uint32_t serial;  // Serial number
        uint8_t idn;      // Send ID in response to *idn? 0=disable, 1=send name; 2=send name+serial
      };
      uint8_t db[GPIB_CFG_SIZE];
    };

    union GPIBconf cfg;

// WORK REQUIRED!
    bool txBreak;  // Signal to break the GPIB transmission
// WORK REQUIRED!

    uint8_t cstate = 0;

    GPIBbus();

    void begin();
    void stop();

    void setDefaultCfg();

    void startControllerMode();
    void startDeviceMode();
    bool isController();

    void sendIFC();
    bool sendLLO();
    bool sendGTL();
    bool sendGET(uint8_t addr);
    bool sendSDC();
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
    uint8_t readByte(uint8_t *db, bool readWithEoi, bool *eoi);  
    uint8_t writeByte(uint8_t db, bool isLastByte);
    bool receiveData(Stream& dataStream, bool detectEoi, bool detectEndByte, uint8_t endByte);
    void sendData(char *data, uint8_t dsize);
    void clearDataBus();
    void setControlVal(uint8_t value, uint8_t mask, uint8_t mode);
    void setDataVal(uint8_t);

//    void setDeviceAddressedState(uint8_t stat);
    bool isDeviceAddressedToListen();
    bool isDeviceAddressedToTalk();
    bool isDeviceInIdleState();

    void signalBreak();

    bool addressDevice(uint8_t addr, bool dir);
    bool unAddressDevice();
    bool haveAddressedDevice();

  private:

    bool deviceAddressed;
    uint8_t deviceAddressedState;
    
//    bool writeByteHandshake(uint8_t db);
//    boolean waitOnPinState(uint8_t state, uint8_t pin, int interval);
    bool isTerminatorDetected(uint8_t bytes[3], uint8_t eorSequence);
    void setSrqSig();
    void clrSrqSig();
};


#endif // AR488_GPIBbus_H
