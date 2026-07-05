#include "RA4M1.h"


//#define GPIO_ACCESS_STD
#define GPIO_ACCESS_FAST



/********************************/
/***** NANO R4 BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvv *****/
#if defined(RA4M1_NANO_R4)

/***** Control pin map *****/
/*
  Data pin map
  ------------
  DIO1_PIN  A0 : GPIB 1  : P
  DIO2_PIN  A1 : GPIB 2  : P
  DIO3_PIN  A2 : GPIB 3  : P
  DIO4_PIN  A3 : GPIB 4  : P
  DIO5_PIN  A4 : GPIB 13 : P
  DIO6_PIN  A5 : GPIB 14 : P
  DIO7_PIN   4 : GPIB 15 : P
  DIO8_PIN   5 : GPIB 16 : P

  Control pin map
  ---------------
  IFC_PIN   8  : GPIB  9 : P : b0
  NDAC_PIN  9  : GPIB  8 : P : b1
  NRFD_PIN  10 : GPIB  7 : P : b2
  DAV_PIN   11 : GPIB  6 : P : b3
  EOI_PIN   12 : GPIB  5 : P : b4
  REN_PIN   3  : GPIB 17 : P : b5
  SRQ_PIN   2  : GPIB 10 : P : b6
  ATN_PIN   7  : GPIB 11 : P : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=LOW, 1=HIGH
    mask : 0=unaffected, 1=affected
*/



/********************************************/
/***** USING FAST GPIO ACCESS FUNCTIONS *****/
/********************************************/
#ifdef GPIO_ACCESS_FAST


//uint8_t lastbyte = 0;


//R_PORT0_Type *port_table[] = { R_PORT0, R_PORT1, R_PORT2, R_PORT3, R_PORT4, R_PORT5, R_PORT6, R_PORT7 };
//static const uint16_t mask_table[] = { 1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7,
//                                       1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12, 1 << 13, 1 << 14, 1 << 15 };

uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };



void pinModeAlt(uint8_t pin, uint8_t mode) {
   
   switch( mode ) {
      case INPUT:
      case INPUT_PULLDOWN:  // TODO: document the INPUT_PULLDOWN is unavailable
         R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT);
         break;
      case INPUT_PULLUP:
         R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT | IOPORT_CFG_PULLUP_ENABLE);
         break;
      case OUTPUT:
      case OUTPUT_OPENDRAIN:
         {
            byte _port = g_pin_cfg[pin].pin >> 8;
            byte _pin = g_pin_cfg[pin].pin & 0xFF;
            bool isHigh = R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PCR != 0;
            uint32_t level = isHigh ? IOPORT_CFG_PORT_OUTPUT_HIGH : IOPORT_CFG_PORT_OUTPUT_LOW;
            if( mode == OUTPUT_OPENDRAIN ) {
               level |= IOPORT_CFG_PMOS_ENABLE;
            }
            R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_OUTPUT | level);
         }
         break;
   }

}



/***** PinMode Fast digital pin *****/

static inline void pinModeFast(pin_size_t pin, uint8_t val) {

  byte _port = g_pin_cfg[pin].pin >> 8;
  byte _pin = g_pin_cfg[pin].pin & 0xFF;

    // Unlock PFS registers
//  R_PMISC->PWPR = 0x00; // Write 0x00 to PWPR_B0WI to disable register write protection.
//  R_PMISC->PWPR = 0x40; // Write 0x40 to PWPR_PFSWE to enable PFS register write access.
//  R_PMISC->PWPR &= ~0x80; // Clear PWPR_B0WI to disable register write protection.
//  R_PMISC->PWPR |= 0x40;  // Set PWPR_PFSWE to enable PFS register write access.
    
  if (val == OUTPUT) {

    R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PDR = 1;  // Set direction OUTPUT
    R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PODR = 1; // Set output HIGH
    R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PCR = 0;  // Disable pull-up
    return;

  }
  
  R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PMR = 0;  // Set direction input
  R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PDR = 0;  // Set direction input
  R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PCR = 1;  // Enable pull-up

  // Re-lock PFS registers
//  R_PMISC->PWPR = 0x00; // Write 0x00 to PWPR_PFSWE to disable PFS register write access.
//  R_PMISC->PWPR = 0x80; // Write 0x80 to re-enable register write protection.
//  R_PMISC->PWPR &= ~0x40; // Clear PWPR_PFSWE to disable PFS register write access.
//  R_PMISC->PWPR |= 0x80;  // Set PWPR_BOWI re-enable register write protection.

}





/***** Fast write digital pin *****/
static inline void digitalWriteFast(pin_size_t pin, uint8_t val) {
/*
  uint16_t port_pin = g_pin_cfg[pin].pin;
  uint16_t pin_mask = mask_table[port_pin & 0x0F];
  port_pin = port_pin >> 8;
*/
/*
  if (val) {
    port_table[port_pin]->POSR = port_table[port_pin]->POSR | pin_mask;
  } else {
    port_table[port_pin]->PORR = port_table[port_pin]->PORR | pin_mask;
  }
*/
/*
  if (val) {
    port_table[port_pin]->PODR |= pin_mask;
  } else {
    port_table[port_pin]->PODR &= ~pin_mask;
  }
*/
  
  byte _port = g_pin_cfg[pin].pin >> 8;
  byte _pin = g_pin_cfg[pin].pin & 0xFF;

  if (val) {
    R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PODR = 1;
  }else{
    R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PODR = 0;
  }
}


/***** Fast read digital pin *****/
static inline uint8_t digitalReadFast(pin_size_t pin) {
/*
  uint16_t port_pin = g_pin_cfg[pin].pin;
  uint16_t pin_mask = mask_table[port_pin & 0x0F];
  port_pin = port_pin >> 8;
  if (port_table[port_pin]->PIDR & pin_mask) {
    return HIGH;
  }else{
    return LOW;
  }
*/

  byte _port = g_pin_cfg[pin].pin >> 8;
  byte _pin = g_pin_cfg[pin].pin & 0xFF;
  
  if (R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PIDR) {
    return HIGH;
  }else{
    return LOW;
  }

}


/***** Fast toggle digital pin *****/
static inline void digitalToggleFast(pin_size_t pin) {
/*
  uint16_t port_pin = g_pin_cfg[pin].pin;
  uint16_t pin_mask = mask_table[port_pin & 0x0F];
  port_pin = port_pin >> 8;
  if (port_table[port_pin]->PODR & pin_mask) {
    port_table[port_pin]->PORR = pin_mask;
  }else{
    port_table[port_pin]->POSR = pin_mask;
  }
*/
  
  byte _port = g_pin_cfg[pin].pin >> 8;
  byte _pin = g_pin_cfg[pin].pin & 0xFF;

  R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PODR ^= 1;
  
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    for (uint8_t i=0; i<8; i++){
      pinModeFast(databus[i], OUTPUT);
      digitalWriteFast(databus[i], HIGH);
    }
    return;
  }
  for (uint8_t i=0; i<8; i++){
    pinModeFast(databus[i], INPUT_PULLUP);
  }
//  lastbyte = 0;
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    if (!digitalReadFast(databus[i])) db += (1<<i);
  }
  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
//  uint8_t changeMask = lastbyte ^ db;
  for (uint8_t i=0; i<8; i++){
//    if (changeMask >> i) {
//        pinModeFast(databus[i], OUTPUT);
        digitalWriteFast(databus[i], ((db&(1<<i)) ? LOW : HIGH) );
//    }
  }
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  // Set pin state
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1<<i)) digitalWriteFast( ctrlbus[i], ((bits&(1<<i)) ? HIGH : LOW) );
  }
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  // Set pin direction
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1<<i)) pinModeFast( ctrlbus[i], ((bits&(1<<i)) ? OUTPUT : INPUT_PULLUP) );
  }
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalReadFast(pin);
}


#endif  // GPIO_ACCESS_FAST




/************************************************/
/***** USING STANDARD GPIO ACCESS FUNCTIONS *****/
/************************************************/
#ifdef GPIO_ACCESS_STD


uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };


/***** Contributed by van_der_decken on the arduino.cc forum *****/
void pinModeAlt(pin_size_t pin, const PinMode mode) {
   
   switch( mode ) {
      case INPUT:
      case INPUT_PULLDOWN:  // TODO: document the INPUT_PULLDOWN is unavailable
         R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT);
         break;
      case INPUT_PULLUP:
         R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_INPUT | IOPORT_CFG_PULLUP_ENABLE);
         break;
      case OUTPUT:
      case OUTPUT_OPENDRAIN:
         {
            byte _port = g_pin_cfg[pin].pin >> 8;
            byte _pin = g_pin_cfg[pin].pin & 0xFF;
            bool isHigh = R_PFS->PORT[_port].PIN[_pin].PmnPFS_b.PCR != 0;
            uint32_t level = isHigh ? IOPORT_CFG_PORT_OUTPUT_HIGH : IOPORT_CFG_PORT_OUTPUT_LOW;
            if( mode == OUTPUT_OPENDRAIN ) {
               level |= IOPORT_CFG_PMOS_ENABLE;
            }
            R_IOPORT_PinCfg(NULL, g_pin_cfg[pin].pin, IOPORT_CFG_PORT_DIRECTION_OUTPUT | level);
         }
         break;
   }
   
}
/***** Contributed by van_der_decken on the arduino.cc forum *****/


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    for (uint8_t i=0; i<8; i++){
      pinModeAlt(databus[i], OUTPUT);
      digitalWrite(databus[i], HIGH);
    }
    return;
  }
  for (uint8_t i=0; i<8; i++){
    pinModeAlt(databus[i], INPUT_PULLUP);
  }
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    if (!digitalRead(databus[i])) db += (1<<i);
  }
  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  for (uint8_t i=0; i<8; i++){
    digitalWrite(databus[i], ((db&(1<<i)) ? LOW : HIGH) );
  }
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1<<i)) digitalWrite( ctrlbus[i], ((bits&(1<<i)) ? HIGH : LOW) );
  }
}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1<<i)) pinModeAlt( ctrlbus[i], ((bits&(1<<i)) ? OUTPUT : INPUT_PULLUP) );
  }
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif  // GPIO_ACCESS_STD



#endif  // RA4M1_NANO_R4
/***** ^^^^^^^^^^^^^^^^^^^^ *****/
/***** NANO R4 BOARD LAYOUT *****/
/********************************/






