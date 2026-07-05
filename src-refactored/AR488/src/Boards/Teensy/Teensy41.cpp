#include "Teensy41.h"

/***********************************/
/***** TEENSY 4.1 BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef IMXRT1062_TEENSY41_01
/*
  Data pin map
  ------------
  DIO1_PIN  23 : GPIB 1  : AD_B1_09 : GPIO1_25
  DIO2_PIN  22 : GPIB 2  : AD_B1_08 : GPIO1_24
  DIO3_PIN  21 : GPIB 3  : AD_B1_11 : GPIO1_27
  DIO4_PIN  20 : GPIB 4  : AD_B1_10 : GPIO1_26
  DIO5_PIN  19 : GPIB 13 : AD_B1_00 : GPIO1_16
  DIO6_PIN  18 : GPIB 14 : AD_B1_01 : GPIO1_17
  DIO7_PIN  17 : GPIB 15 : AD_B1_06 : GPIO1_22
  DIO8_PIN  16 : GPIB 16 : AD_B1_07 : GPIO1_23

  Control pin map
  ---------------
  IFC_PIN   15 : GPIB 9  : AD_B1_03 : GPIO1_19 : b0
  NDAC_PIN  14 : GPIB 8  : AD_B1_02 : GPIO1_18 : b1
  NRFD_PIN  41 : GPIB 7  : AD_B1_05 : GPIO1_21 : b2
  DAV_PIN   40 : GPIB 6  : AD_B1_04 : GPIO1_20 : b3
  EOI_PIN   39 : GPIB 5  : AD_B1_13 : GPIO1_29 : b4

  REN_PIN   26 : GPIB 17 : AD_B1_14 : GPIO1_30 : b5
  SRQ_PIN   38 : GPIB 10 : AD_B1_12 : GPIO1_28 : b6
  ATN_PIN   27 : GPIB 11 : AD_B1_15 : GPIO1_31 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


/***************************************/
/*****   TEENSY NATIVE FUNCTIONS   *****/
/***************************************/

#ifdef TEENSY_NATIVE_FUNCS

/*
 * While the GPIO direction is set to input, (GDIR[GDIR]=0), a
 * read access to DR does not return DR data. Instead it returns
 * the PSR data, which is the corresponding input signal value.
 *
 * GDIR - direction register
 * DR   - data register
 * PSR  - pad sample register
*/

/* 
 * middle hook example to set pin states early
 * FLASHMEM tells the compiler/linker to leave that bit of code in flash and execute it from there.
 *
 * extern "C" void startup_middle_hook(void);
 * FLASHMEM void startup_middle_hook(void) {
 *   pinMode(24, OUTPUT);
 *   digitalWrite(24, HIGH);
 * }
*/

/***** GPIB data and control buses ******/

uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };
uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };

const uint8_t databusTsy[2][8] = {
                { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN },
                { 25, 24, 27, 26, 16, 17, 22, 23 }  // Corresponding GPIO6 bits
};

const uint8_t ctrlbusTsy[2][8] = {
                { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN },
                { 19, 18, 21, 20, 29, 30, 28, 31 }  // Corresponding GPIO6 bits
};


/***** GPIO6 register value *****/
uint32_t gpioDbReg = 0;
uint32_t gpioctrlbus = 0;

volatile uint32_t * iomuxcPadGpio6[] = {
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_00,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_01,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_02,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_03,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_04,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_05,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_06,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_07,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_08,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_09,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_10,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_11,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_12,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_13,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_14,
  &IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B1_15
};

volatile uint32_t * iomuxcMuxGpio6[] = {
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_00,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_01,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_02,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_03,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_04,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_05,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_06,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_07,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_08,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_09,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_10,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_11,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_12,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_13,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_14,
  &IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B1_15
};


/***** Generate GPIO mask from assigned pin map *****/
uint64_t genGpioMask(const uint8_t buspins[][8], uint8_t bitmask) {
  uint64_t gpioreg = 0;
  for (uint8_t i=0; i<8; i++) {
    if (bitmask & (1 << i)) {
      gpioreg |= ( 1ULL << buspins[1][i] );
    }
  }
  return gpioreg;
}


/***** Set pullups on all bus pins *****/
void setBusPullups(const uint8_t bus[][8], bool enable) {
  volatile uint32_t * regptr = 0;
  if (enable) {
    for (uint8_t i=0; i<8; i++){
      regptr = iomuxcPadGpio6[ (bus[1][i] - 16) ];
      *regptr |= PIN_PULLUP_ENABLE;
    }
    return;
  }
  for (uint8_t i=0; i<8; i++){
    regptr = iomuxcPadGpio6[ (bus[1][i] - 16) ];
    *regptr &= PIN_PULLUP_DISABLE;  
  }
}


/***** Set SION bit on all bus pins *****/
void setBusSionBit(const uint8_t bus[][8], bool enable) {
  volatile uint32_t * regptr = 0;
  if (enable) {
    for (uint8_t i=0; i<8; i++){
      regptr = iomuxcMuxGpio6[ (bus[1][i] - 16) ];
      *regptr |= PIN_SION_ENABLE; 
    }
    return;
  }
  for (uint8_t i=0; i<8; i++){
    regptr = iomuxcMuxGpio6[ (bus[1][i] - 16) ];
    *regptr &= PIN_SION_DISABLE;    
  }
}


/***** Print 32-bit register *****/
void printReg(const char * regname, uint32_t regvalue){
  char buffer[64];
  char valbuf[34]; 
  
  memset(buffer, '\0', sizeof(buffer));
  memset(valbuf, '\0', sizeof(valbuf));
  memset(valbuf, 0x30, 32);

  for (uint8_t i=0; i<32; i++) {
    if (regvalue & (1UL<<i)) valbuf[31-i]++;
  }
  
  sprintf(buffer, "%-10s: %s", regname, valbuf);
  Serial.println(buffer);
}


/***** Initialise GPIO pins *****/
void initTsyGpioPins(){

  uint32_t gpioall;

  // Generate mask that identifies bits for each bus
  gpioDbReg = genGpioMask(databusTsy, 0xFF);
  gpioctrlbus = genGpioMask(ctrlbusTsy, 0xFF);
  gpioall = (gpioDbReg | gpioctrlbus);
  
  // Set all required GPIOs to input_pullup
  GPIO1_GDIR &= ~gpioall;
  setBusSionBit(databusTsy, true);
  setBusSionBit(ctrlbusTsy, true);
  setBusPullups(databusTsy, true);
  setBusPullups(ctrlbusTsy, true);

}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {

  if (state == OUTPUT) {
    GPIO6_GDIR |= gpioDbReg;
    setBusPullups(databusTsy, false);
    GPIO6_DR |= gpioDbReg;    // Outputs high by default
    return;
  }

  GPIO6_GDIR &= ~gpioDbReg;
  setBusPullups(databusTsy, true);

}


/***** Read the GPIB data bus wires to collect the byte of data *****/

uint8_t readGpibDbus() {
  uint8_t db = 0;
  uint32_t reg = GPIO6_PSR;
  
  for (uint8_t i=0; i<8; i++){
    if ( reg & (1UL<<databusTsy[1][i]) ) db |= (1U<<i);
  }
  
  return ~db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/

void setGpibDbus(uint8_t db) {

  uint32_t gpioval = genGpioMask(databusTsy, ~db);

  GPIO6_GDIR |= gpioDbReg;            // Set as outputs
  GPIO6_DR |= gpioDbReg;              // Set high
  setBusPullups(databusTsy, false);   // Disable pull-ups  
  GPIO6_DR &= (gpioval | ~gpioDbReg); // Set value

}


/***** Set control pin output level *****/
void setGpibCtrlState(uint8_t bits, uint8_t mask) {
  uint32_t gpiobit = 0;
  for (uint8_t i=0; i<8; i++) {    
    gpiobit = (1UL<<ctrlbusTsy[1][i]);
    if (mask&(1U<<i)) {
      if (bits&(1U<<i)) {
        GPIO6_DR |= gpiobit;
      }else{
        GPIO6_DR &= ~gpiobit;
      }
    }
  }
}


/***** Set control pin direction *****/
void setGpibCtrlDir(uint8_t bits, uint8_t mask) {
  volatile uint32_t * regptr = 0;
  uint32_t gpiobit = 0;
  uint8_t idx = 0;
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1U<<i)) {
      gpiobit = (1UL<<ctrlbusTsy[1][i]);
      idx = ctrlbusTsy[1][i] - 16;
      if (bits&(1U<<i)) {
        // Output
        GPIO6_GDIR |= gpiobit;
        regptr = iomuxcPadGpio6[idx];
        *regptr &= PIN_PULLUP_DISABLE;
      }else{
        // Input_pullup
        GPIO6_GDIR &= ~gpiobit;
        regptr = iomuxcPadGpio6[idx];
        *regptr |= PIN_PULLUP_ENABLE;
      }
    }    
  }
}


uint8_t getGpibPinState(uint8_t pin){
//  return digitalRead(pin);
  return digitalReadFast(pin);
}


#endif  // TEENSY_NATIVE_FUNCS




/******************************************/
/*****   ARDUINO STANDARD FUNCTIONS   *****/
/******************************************/

#ifdef TEENSY_ARDUINO_FUNCS

uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };



void initTsyGpioPins() {
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    for (uint8_t i=0; i<8; i++){
      pinMode(databus[i], OUTPUT);
      // Set outputs to high?
      digitalWrite(databus[i], HIGH);
    }
  }

  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], INPUT_PULLUP);
  }
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint8_t db = 0;
  for (uint8_t i=0; i<8; i++){
    db = db + (digitalRead(databus[i]) ? 0 : 1U<<i );
  }
  return db;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  for (uint8_t i=0; i<8; i++){
    pinMode(databus[i], OUTPUT);
    digitalWrite(databus[i], ((db&(1U<<i)) ? LOW : HIGH) );
  }
  
}


void setGpibCtrlState(uint8_t bits, uint8_t mask) {

  // Set pin state
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1U<<i)) digitalWrite( ctrlbus[i], ((bits&(1U<<i)) ? HIGH : LOW) );
  }

}


void setGpibCtrlDir(uint8_t bits, uint8_t mask) {

  // Set pin direction
  for (uint8_t i=0; i<8; i++) {
    if (mask&(1U<<i)) pinMode( ctrlbus[i], ((bits&(1U<<i)) ? OUTPUT : INPUT_PULLUP) );
  }
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}


#endif  // TEENSY_ARDUINO_FUNCS


#endif // IMXRT1062_TEENSY41_01
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** TEENSY 4.1 BOARD LAYOUT *****/
/***********************************/


