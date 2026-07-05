#include "ESP32.h"



/************************************/
/*****  ESP32_HYBRID_FUNCTIONS  *****/
/************************************/

#if defined(ESP32_NATIVE_FUNC)

/***** Struct used to hold GPIO register values *****/
struct gpioregister_t {
  uint32_t reg0 = 0;
  uint32_t reg1 = 0;
};


/***** Array holding pin map *****/
const uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };
const uint8_t ctrlreg[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };


/***** 64-bit control and data bus pin masks *****/
uint64_t gpioDbMask = 0;
uint64_t gpioCtrlMask = 0;


/***** ESP GPIO configuratiom object *****/
gpio_config_t gpioCfg;
const gpio_config_t * gpioCfgPtr = &gpioCfg;


/***** Covert 64-bit mask to 2 x 32-bit regster values *****/
void mask64ToReg(gpioregister_t& gpioreg, uint64_t mask) {
  gpioreg.reg0 = (mask & 0xFFFFFFFF);
  gpioreg.reg1 = (mask >> 32);
}


/***** Convert 2 x 32-bit register values to 64-bit mask *****/
uint64_t regToMask64(gpioregister_t& gpioreg) {
  uint64_t gpiomask = 0;
  gpiomask = gpioreg.reg0;
  gpiomask |= ((uint64_t)gpioreg.reg1 << 32);
  return gpiomask;
}


/***** Generate GPIO mask from assigned pin map *****/
uint64_t genGpioMask(const uint8_t buspins[], uint8_t bitmask) {
  uint64_t gpioreg = 0;
  for (uint8_t i=0; i<8; i++) {
    if (bitmask & (1 << i)) {
      gpioreg |= ( 1ULL << buspins[i] );
    }
  }
  return gpioreg;
}


/***** Set the direction of GPIO pins using a mask *****/
/*
void setGpioDirMasked(const uint8_t bus[], uint8_t mask, uint8_t state = INPUT_PULLUP) {

  // OUTPUT mode
  if (state == OUTPUT) {
    for (uint8_t i=0; i<8; i++) {
      if ( mask & (1U << i) ) pinMode(bus[i], OUTPUT);
    }
    return;
  }

  // INPUT_PULLUP mode
  for (uint8_t i=0; i<8; i++) {
    if ( mask & (1U << i) ) pinMode(bus[i], INPUT_PULLUP);
  }

}
*/


void setGpioDirMasked(const uint8_t bus[], uint8_t mask, uint8_t state = INPUT_PULLUP) {

  uint64_t gpiomask = 0;

  for (uint8_t i=0; i<8; i++){
    if ( mask & (1U << i) ) gpiomask |= (1ULL<<bus[i]);
  }

  if (state == OUTPUT) {
  // OUTPUT mode
    gpioCfg.pin_bit_mask = gpiomask;
    gpioCfg.mode = GPIO_MODE_INPUT_OUTPUT;
    gpioCfg.pull_up_en = GPIO_PULLUP_DISABLE;
  }else{
    // INPUT_PULLUP mode
    gpioCfg.pin_bit_mask = gpiomask;
    gpioCfg.mode = GPIO_MODE_INPUT;
    gpioCfg.pull_up_en = GPIO_PULLUP_ENABLE;
  }

  gpio_config(gpioCfgPtr);

}



/***** Read a GPIO pin using in/out register OR'ing *****/
/*
uint8_t digitalReadReg(uint8_t pin){
  if (pin>=SOC_GPIO_PIN_COUNT) return 0;   // Pins above 48 are invalid
  uint64_t gpioall = 0;
  gpioregister_t reg;
  reg.reg0 = REG_READ(GPIO_IN_REG);
  reg.reg1 = REG_READ(GPIO_IN1_REG);
  gpioall = regToMask64(reg);
  reg.reg0 = REG_READ(GPIO_OUT_REG);
  reg.reg1 = REG_READ(GPIO_OUT1_REG);
  gpioall |= regToMask64(reg);
  if ( gpioall & (1ULL<<pin) ) return 1;
  return 0;
}
*/
/*
uint8_t digitalReadReg(uint8_t pin){
  if (pin>SOC_GPIO_PIN_COUNT) return 0;   // Pins above 48 are invalid
  uint64_t gpioall = 0;
  gpioregister_t reg;
  bool pinoutp = false;

  reg.reg0 = REG_READ(GPIO_ENABLE_REG);
#ifdef GPIO_ENABLE1_REG
  reg.reg1 = REG_READ(GPIO_ENABLE1_REG);
#endif
  gpioall = regToMask64(reg);
  if ( gpioall & (1ULL<<pin) ) pinoutp = true;
  
  if (pinoutp) {
    reg.reg0 = REG_READ(GPIO_OUT_REG);
#ifdef GPIO_OUT1_REG
    reg.reg1 = REG_READ(GPIO_OUT1_REG);
#endif
    gpioall = regToMask64(reg);
    if ( gpioall & (1ULL<<pin) ) return 1;
    return 0;
  }

  reg.reg0 = REG_READ(GPIO_IN_REG);
#ifdef GPIO_IN1_REG
  reg.reg1 = REG_READ(GPIO_IN1_REG);
#endif
  gpioall |= regToMask64(reg);
  if ( gpioall & (1ULL<<pin) ) return 1;
  return 0;
}
*/



uint8_t digitalReadReg(uint8_t pin){
  if (pin>SOC_GPIO_PIN_COUNT) return 0;   // Pins above 48 are invalid
  if (GPIO_IS_VALID_GPIO(pin)) {
    uint64_t gpioall = 0;
    gpioregister_t reg;
  
    reg.reg0 = REG_READ(GPIO_IN_REG);
#ifdef GPIO_OUT1_REG
    reg.reg1 = REG_READ(GPIO_IN1_REG);
#endif
    gpioall = regToMask64(reg);
    if ( gpioall & (1ULL<<pin) ) return 1;
  }
  return 0;
}


/***** Init GPIO registers *****/
void initEspGpioPins(){
  // Generate masks
  gpioDbMask = genGpioMask(databus, 0xFF);
  gpioCtrlMask = genGpioMask(ctrlreg, 0xFF);

  // Configure all GPIOs to input pullup (default?)
  uint64_t gpioall = (gpioDbMask | gpioCtrlMask);  
  gpioCfg.pin_bit_mask = gpioall;
  gpioCfg.mode = GPIO_MODE_INPUT;
  gpioCfg.pull_up_en = GPIO_PULLUP_ENABLE;
  gpioCfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpioCfg.intr_type =  GPIO_INTR_DISABLE;
  gpio_config(gpioCfgPtr);
  
}


/**** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {

  gpioregister_t gpiodb;
  
  mask64ToReg(gpiodb, gpioDbMask);
 
  // Set pins to OUTPUT
  if (state == OUTPUT) {
    setGpioDirMasked(databus, 0xFF, OUTPUT);
    // Set to HIGH
    REG_WRITE(GPIO_OUT_W1TS_REG, gpiodb.reg0);
#ifdef GPIO_OUT1_W1TS_REG
    REG_WRITE(GPIO_OUT1_W1TS_REG, gpiodb.reg1);
#endif
    return;
  }
  
  // Set pins to INPUT_PULLUP
  setGpioDirMasked(databus, 0xFF, INPUT_PULLUP);

}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  uint64_t gpioall = 0;
  gpioregister_t gpioreg;
  
  // Read the byte of data on the bus`
  gpioreg.reg0 = REG_READ(GPIO_IN_REG);
  gpioreg.reg1 = REG_READ(GPIO_IN1_REG);
  gpioall = regToMask64(gpioreg);

  // Calculate and return result
  uint8_t result = 0;
  for (uint8_t i=0; i<8; i++) {
    if ( gpioall & (1ULL<<databus[i]) ) result = (result | (1U<<i));
  }
  return ~result;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  uint64_t busmask = genGpioMask(databus, 0xFF);
  uint64_t valmask = genGpioMask(databus, db);
  gpioregister_t busreg;
  gpioregister_t valreg;
  mask64ToReg(busreg, busmask);
  mask64ToReg(valreg, valmask);
  
  // Set databus to output
  readyGpibDbus(OUTPUT);
  
  // Set required value
  if (db>0) {
    REG_WRITE( GPIO_OUT_W1TC_REG, valreg.reg0 );
#ifdef GPIO_OUT1_W1TC_REG
    REG_WRITE( GPIO_OUT1_W1TC_REG, valreg.reg1 );
#endif
  }
}


/***** Set GPIB ctrl pin state *****/
void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint8_t hbits = bits & mask;
  uint8_t lbits = ~(bits | ~mask);

  uint64_t gpiomask = 0;

  if (hbits) {
    gpioregister_t gpioHbits;
    gpiomask = genGpioMask(ctrlreg, hbits);
    mask64ToReg(gpioHbits, gpiomask);

    if (gpioHbits.reg0) REG_WRITE( GPIO_OUT_W1TS_REG, gpioHbits.reg0 );
#ifdef GPIO_OUT1_W1TS_REG
    if (gpioHbits.reg1) REG_WRITE( GPIO_OUT1_W1TS_REG, gpioHbits.reg1 );
#endif
  }
  
  if (lbits){
    gpioregister_t gpioLbits;
    gpiomask = genGpioMask(ctrlreg, lbits);
    mask64ToReg(gpioLbits, gpiomask);

    if (gpioLbits.reg0) REG_WRITE( GPIO_OUT_W1TC_REG, gpioLbits.reg0 );
#ifdef GPIO_OUT1_W1TC_REG
    if (gpioLbits.reg1) REG_WRITE( GPIO_OUT1_W1TC_REG, gpioLbits.reg1 );
#endif
  }
}


/***** Set the direction of control pins *****/

void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint8_t obits = bits & mask;
  uint8_t ibits =  ~(bits | ~mask);


  if (obits) setGpioDirMasked(ctrlreg, obits, OUTPUT);

  if (ibits) setGpioDirMasked(ctrlreg, ibits, INPUT_PULLUP);
  
}


uint8_t getGpibPinState(uint8_t pin){
//  return digitalRead(pin);
  return digitalReadReg(pin);
}

#endif      // ESP32_NATIVE_FUNC






/*********************************/
/*****  ESP32 ARDUINO FUNC   *****/
/***** vvvvvvvvvvvvvvvvvvvvv *****/


#ifdef ESP32_ARDUINO_FUNC

uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };


/***** Dummy function *****/
void initEspGpioPins(){
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    for (uint8_t i=0; i<8; i++){
      pinMode(databus[i], OUTPUT);
      digitalWrite(databus[i], HIGH);
    }
    return;
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


#endif  // ESP32_ARDUINO_FUNC
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ESP32 ARDUINO FUNCTIONS *****/
/***********************************/

