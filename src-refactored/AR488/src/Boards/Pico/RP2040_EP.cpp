#include "RP2040_EP.h"

/***********************************/
/***** RAS PICO BOARD LAYOUT 1 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L1
/*
  Data pin map
  ------------
  DIO1_PIN   6 : GPIB 1  : GPIO6
  DIO2_PIN   7 : GPIB 2  : GPIO7
  DIO3_PIN   8 : GPIB 3  : GPIO8
  DIO4_PIN   9 : GPIB 4  : GPIO9
  DIO5_PIN  10 : GPIB 13 : GPIO10
  DIO6_PIN  11 : GPIB 14 : GPIO11
  DIO7_PIN  12 : GPIB 15 : GPIO12
  DIO8_PIN  13 : GPIB 16 : GPIO13

  Control pin map
  ---------------
  IFC_PIN   14 : GPIB 9  : GPIO14 : b0
  NDAC_PIN  15 : GPIB 8  : GPIO15 : b1
  NRFD_PIN  16 : GPIB 7  : GPIO16 : b2
  DAV_PIN   17 : GPIB 6  : GPIO17 : b3
  EOI_PIN   18 : GPIB 5  : GPIO18 : b4
  REN_PIN   19 : GPIB 17 : GPIO19 : b5
  SRQ_PIN   20 : GPIB 10 : GPIO20 : b6
  ATN_PIN   21 : GPIB 11 : GPIO21 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


const uint32_t gpioDbMask = 0x00003FC0;
const uint32_t gpioCtrlMask = 0x003FC000;
const uint8_t gpioDbOffset = 6;
const uint8_t gpioCtrlOffset = 14;


#ifdef USE_RP2040_FUNC_LIST
void gpioFuncList(){
  Serial.println();
  Serial.print(F("Pull: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  Serial.println();
  Serial.print(F("Outs: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  Serial.println();
}
#endif


/***** Set GPIO internal pullup resistors *****/
void gpio_set_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_pull_up(i);
  }
}


/***** Disable GPIO pull-up resistors *****/
void gpio_clear_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_disable_pulls(i);
  }
}


/***** Initialise all GPIO pins *****/
void initRpGpioPins(){
  uint32_t gpiomask = gpioCtrlMask | gpioDbMask;  // Scope of GPIO pins to be allocated to GPIB
  gpio_init_mask(gpiomask);                       // Configure as GPIO pins those allocated to GPIB
  gpio_set_dir_in_masked(gpiomask);               // Configure all as inputs by default
  gpio_set_pullups_masked(gpiomask);              // Enable all pullups (un-asserted) by default
  delayMicroseconds(50);                          // Allow some settling time
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    gpio_set_dir_out_masked(gpioDbMask);
    // Set outputs to high
    gpio_put_masked(gpioDbMask, gpioDbMask);
    return;
  }

  // Set data pins to input
  gpio_set_dir_in_masked(gpioDbMask);
  gpio_set_pullups_masked(gpioDbMask);
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  gpioall = (gpioall & gpioDbMask) >> gpioDbOffset;
  return (uint8_t)~gpioall;  
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  unsigned long gpioall = ( (~db & 0xFF) << gpioDbOffset );
  gpio_clear_pullups_masked(gpioDbMask);
  gpio_set_dir_out_masked(gpioDbMask);
  gpio_put_masked(gpioDbMask, gpioall);
}


/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    - bits : 0=LOW, 1=HIGH
    - mask : 0=unaffected, 1=affected
    Has relevance only to output pins
*/
void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint32_t gpiobits = ( ((bits & mask) & 0xFF) << gpioCtrlOffset );
  uint32_t gpioOmask = (mask << gpioCtrlOffset);

  gpio_put_masked(gpioOmask, gpiobits);

}


/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=input pullup, 1=output
    mask : 0=unaffected, 1=affected
*/
void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint32_t gpioOmask = ( ((bits & mask) & 0xFF) << gpioCtrlOffset );
  uint32_t gpioImask = ( ((~bits & mask) & 0xFF) << gpioCtrlOffset );

  if (gpioOmask){
    gpio_clear_pullups_masked(gpioOmask);
    gpio_set_dir_out_masked(gpioOmask);
  }

  if (gpioImask){
    gpio_set_pullups_masked(gpioImask);
    gpio_set_dir_in_masked(gpioImask);
  }
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}


#endif // RAS_PICO_L1
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO BOARD LAYOUT 1 *****/
/***********************************/



/***********************************/
/***** RAS PICO BOARD LAYOUT 2 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L2
/*
  Data pin map
  ------------
  DIO1_PIN  14 : GPIB 1  : GPIO14
  DIO2_PIN  15 : GPIB 2  : GPIO15
  DIO3_PIN  16 : GPIB 3  : GPIO16
  DIO4_PIN  17 : GPIB 4  : GPIO17
  DIO5_PIN  18 : GPIB 13 : GPIO18
  DIO6_PIN  19 : GPIB 14 : GPIO19
  DIO7_PIN  20 : GPIB 15 : GPIO20
  DIO8_PIN  21 : GPIB 16 : GPIO21

  Control pin map
  ---------------
  IFC_PIN    6 : GPIB 9  : GPIO6  : b0
  NDAC_PIN   7 : GPIB 8  : GPIO7  : b1
  NRFD_PIN   8 : GPIB 7  : GPIO8  : b2
  DAV_PIN    9 : GPIB 6  : GPIO9  : b3
  EOI_PIN   10 : GPIB 5  : GPIO10 : b4
  REN_PIN   11 : GPIB 17 : GPIO11 : b5
  SRQ_PIN   12 : GPIB 10 : GPIO12 : b6
  ATN_PIN   13 : GPIB 11 : GPIO13 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


const uint32_t gpioDbMask = 0x003FC000;
const uint32_t gpioCtrlMask = 0x00003FC0;
const uint8_t gpioDbOffset = 14;
const uint8_t gpioCtrlOffset = 6;


#ifdef USE_RP2040_FUNC_LIST
void gpioFuncList(){
  Serial.println();
  Serial.print(F("Pull: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  Serial.println();
  Serial.print(F("Outs: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  Serial.println();
}
#endif


/***** Set GPIO internal pullup resistors *****/
void gpio_set_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_pull_up(i);
  }
}


/***** Disable GPIO pull-up resistors *****/
void gpio_clear_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_disable_pulls(i);
  }
}

/*
uint8_t reverseBits(uint8_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}
*/

/***** Initialise all GPIO pins *****/
void initRpGpioPins(){
  uint32_t gpiomask = gpioCtrlMask | gpioDbMask;  // Scope of GPIO pins to be allocated to GPIB
  gpio_init_mask(gpiomask);                       // Configure as GPIO pins those allocated to GPIB
  gpio_set_dir_in_masked(gpiomask);               // Configure all as inputs by default
  gpio_set_pullups_masked(gpiomask);              // Enable all pullups (un-asserted) by default
  delayMicroseconds(50);                          // Allow some settling time
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    gpio_set_dir_out_masked(gpioDbMask);
    // Set outputs to high
    gpio_put_masked(gpioDbMask, gpioDbMask);
    return;
  }

  // Set data pins to input
  gpio_set_dir_in_masked(gpioDbMask);
  gpio_set_pullups_masked(gpioDbMask);
  
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  gpioall = (gpioall & gpioDbMask) >> gpioDbOffset;
  return (uint8_t)~gpioall;  
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint32_t gpioall = ( (~db & 0xFF) << gpioDbOffset );
  gpio_clear_pullups_masked(gpioDbMask);
  gpio_set_dir_out_masked(gpioDbMask);
  gpio_put_masked(gpioDbMask, gpioall);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint32_t gpiobits = ( ((bits & mask) & 0xFF) << gpioCtrlOffset );
  uint32_t gpioOmask = (mask << gpioCtrlOffset);

  gpio_put_masked(gpioOmask, gpiobits);

}


/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=input pullup, 1=output
    mask : 0=unaffected, 1=affected
*/
void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint32_t gpioOmask = ( ((bits & mask) & 0xFF) << gpioCtrlOffset );
  uint32_t gpioImask = ( ((~bits & mask) & 0xFF) << gpioCtrlOffset );

  if (gpioOmask){
    gpio_clear_pullups_masked(gpioOmask);
    gpio_set_dir_out_masked(gpioOmask);
  }

  if (gpioImask){
    gpio_set_pullups_masked(gpioImask);
    gpio_set_dir_in_masked(gpioImask);
  }
  
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // RAS_PICO_L2
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO BOARD LAYOUT 2 *****/
/***********************************/



/***********************************/
/***** RAS PICO BOARD LAYOUT 3 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L3
/*
  Data pin map
  ------------
  DIO1_PIN   2 : GPIB 1  : GPIO14
  DIO2_PIN   3 : GPIB 2  : GPIO15
  DIO3_PIN   4 : GPIB 3  : GPIO16
  DIO4_PIN   5 : GPIB 4  : GPIO17
  DIO5_PIN   6 : GPIB 13 : GPIO18
  DIO6_PIN   7 : GPIB 14 : GPIO19
  DIO7_PIN   8 : GPIB 15 : GPIO20
  DIO8_PIN   9 : GPIB 16 : GPIO21

  Control pin map
  ---------------
  IFC_PIN   10 : GPIB 9  : GPIO6  : b0
  NDAC_PIN  11 : GPIB 8  : GPIO7  : b1
  NRFD_PIN  12 : GPIB 7  : GPIO8  : b2
  DAV_PIN   13 : GPIB 6  : GPIO9  : b3
  EOI_PIN   14 : GPIB 5  : GPIO10 : b4
  REN_PIN   15 : GPIB 17 : GPIO11 : b5
  SRQ_PIN   20 : GPIB 10 : GPIO12 : b6
  ATN_PIN   21 : GPIB 11 : GPIO13 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


const uint32_t gpioDbMask = 0x000003FC;
const uint32_t gpioCtrlMask = 0x0030FC00;
const uint8_t gpioDbOffset = 2;
const uint8_t gpioCtrlOffset = 10;


#ifdef USE_RP2040_FUNC_LIST
void gpioFuncList(){
  Serial.println();
  Serial.print(F("Pull: "));
  for (uint8_t i=21; i>19; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  for (uint8_t i=15; i>1; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  Serial.println();
  Serial.print(F("Outs: "));
  for (uint8_t i=21; i>19; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  for (uint8_t i=15; i>1; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  Serial.println();
}
#endif


/***** Set GPIO internal pullup resistors *****/
void gpio_set_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_pull_up(i);
  }
}


/***** Disable GPIO pull-up resistors *****/
void gpio_clear_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_disable_pulls(i);
  }
}


/***** Initialise all GPIO pins *****/
void initRpGpioPins(){
  uint32_t gpiomask = gpioCtrlMask | gpioDbMask;  // Scope of GPIO pins to be allocated to GPIB
  gpio_init_mask(gpiomask);                       // Configure as GPIO pins those allocated to GPIB
  gpio_set_dir_in_masked(gpiomask);               // Configure all as inputs by default
  gpio_set_pullups_masked(gpiomask);              // Enable all pullups (un-asserted) by default
  delayMicroseconds(200);                          // Allow some settling time
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    gpio_set_dir_out_masked(gpioDbMask);
    // Set outputs to high
    gpio_put_masked(gpioDbMask, gpioDbMask);
    return;
  }

  // Set data pins to input
  gpio_set_dir_in_masked(gpioDbMask);
  gpio_set_pullups_masked(gpioDbMask);
  
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  gpioall = (gpioall & gpioDbMask) >> gpioDbOffset;
  return (uint8_t)~gpioall;  
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint32_t gpioall = ((uint8_t)~db) << gpioDbOffset;
  gpio_clear_pullups_masked(gpioDbMask);
  gpio_set_dir_out_masked(gpioDbMask);
  gpio_put_masked(gpioDbMask, gpioall);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint8_t masked = bits & mask;

  uint32_t gpiobits = ( ((masked & 0x3F) << gpioCtrlOffset) | ((masked & 0xC0) << (gpioCtrlOffset + 4) ) );
  uint32_t gpiomask = ( ((mask & 0x3F) << gpioCtrlOffset) | ((mask & 0xC0) << (gpioCtrlOffset + 4)) );

  gpio_put_masked(gpiomask, gpiobits);

}


/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=input pullup, 1=output
    mask : 0=unaffected, 1=affected
*/
void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint8_t masked = bits & mask;
  uint8_t rmasked = ~bits & mask;

  uint32_t gpioOmask = ( ((masked & 0x3F) << gpioCtrlOffset) | ((masked & 0xC0) << (gpioCtrlOffset + 4) ) );
  uint32_t gpioImask = ( ((rmasked & 0x3F) << gpioCtrlOffset) | ((rmasked & 0xC0) << (gpioCtrlOffset + 4) ) );

  if (gpioOmask){
    gpio_clear_pullups_masked(gpioOmask);
    gpio_set_dir_out_masked(gpioOmask);
  }

  if (gpioImask){
    gpio_set_pullups_masked(gpioImask);
    gpio_set_dir_in_masked(gpioImask);
  }
  
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}


#endif // RAS_PICO_L3
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO BOARD LAYOUT 3 *****/
/***********************************/



/***********************************/
/***** RAS PICO BOARD LAYOUT 4 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_L4
/*
  Data pin map
  ------------
  DIO1_PIN  10 : GPIB 10 : GPIO14
  DIO2_PIN  11 : GPIB 11 : GPIO15
  DIO3_PIN  12 : GPIB 12 : GPIO16
  DIO4_PIN  13 : GPIB 13 : GPIO17
  DIO5_PIN  14 : GPIB 14 : GPIO18
  DIO6_PIN  15 : GPIB 15 : GPIO19
  DIO7_PIN  20 : GPIB 20 : GPIO20
  DIO8_PIN  21 : GPIB 21 : GPIO21

  Control pin map
  ---------------
  IFC_PIN    2 : GPIB 9  : GPIO6  : b0
  NDAC_PIN   3 : GPIB 8  : GPIO7  : b1
  NRFD_PIN   4 : GPIB 7  : GPIO8  : b2
  DAV_PIN    5 : GPIB 6  : GPIO9  : b3
  EOI_PIN    6 : GPIB 5  : GPIO10 : b4
  REN_PIN    7 : GPIB 17 : GPIO11 : b5
  SRQ_PIN    8 : GPIB 10 : GPIO12 : b6
  ATN_PIN    9 : GPIB 11 : GPIO13 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


const uint32_t gpioDbMask = 0x0030FC00;
const uint32_t gpioCtrlMask = 0x000003FC;
const uint8_t gpioDbOffset = 10;
const uint8_t gpioCtrlOffset = 2;


#ifdef USE_RP2040_FUNC_LIST
void gpioFuncList(){
  Serial.println();
  Serial.print(F("Pull: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  Serial.println();
  Serial.print(F("Outs: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  Serial.println();
}
#endif


/***** Set GPIO internal pullup resistors *****/
void gpio_set_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_pull_up(i);
  }
}


/***** Disable GPIO pull-up resistors *****/
void gpio_clear_pullups_masked(uint32_t mask){
  for (uint8_t i=0; i<32; i++){
    if ( mask & (1<<i) ) gpio_disable_pulls(i);
  }
}


/***** Initialise all GPIO pins *****/
void initRpGpioPins(){
  uint32_t gpiomask = gpioCtrlMask | gpioDbMask;  // Scope of GPIO pins to be allocated to GPIB
  gpio_init_mask(gpiomask);                       // Configure as GPIO pins those allocated to GPIB
  gpio_set_dir_in_masked(gpiomask);               // Configure all as inputs by default
  gpio_set_pullups_masked(gpiomask);              // Enable all pullups (un-asserted) by default
  delayMicroseconds(50);                          // Allow some settling time
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
    gpio_set_dir_out_masked(gpioDbMask);
    // Set outputs to high
    gpio_put_masked(gpioDbMask, gpioDbMask);
    return;
  }

  // Set data pins to input
  gpio_set_dir_in_masked(gpioDbMask);
  gpio_set_pullups_masked(gpioDbMask);
  
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  gpioall = ((gpioall & 0x0000FC00) >> gpioDbOffset) | ((gpioall & 0x00300000) >> (gpioDbOffset + 4));
  return (uint8_t)~gpioall;  
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint8_t rdb = ~db;
  uint32_t gpioall = ( ((rdb & 0x3F) << gpioDbOffset) | ((rdb & 0xC0) << (gpioDbOffset + 4)) );
  gpio_clear_pullups_masked(gpioDbMask);
  gpio_set_dir_out_masked(gpioDbMask);
  gpio_put_masked(gpioDbMask, gpioall);
}


void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint8_t maskedbits = bits & mask;
  uint32_t gpiobits = (maskedbits << gpioCtrlOffset);
  uint32_t gpiomask = (mask << gpioCtrlOffset);

  gpio_put_masked(gpiomask, gpiobits);

}


/*
   Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits : 0=input pullup, 1=output
    mask : 0=unaffected, 1=affected
*/
void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint8_t masked = bits & mask;
  uint8_t rmasked = ~bits & mask;

  uint32_t gpioOmask = ( masked << gpioCtrlOffset );
  uint32_t gpioImask = ( rmasked << gpioCtrlOffset );

  if (gpioOmask){
    gpio_clear_pullups_masked(gpioOmask);
    gpio_set_dir_out_masked(gpioOmask);
  }

  if (gpioImask){
    gpio_set_pullups_masked(gpioImask);
    gpio_set_dir_in_masked(gpioImask);
  }
  
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // RAS_PICO_L4
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO BOARD LAYOUT 4 *****/
/***********************************/



/***********************************/
/***** RAS PICO BOARD LAYOUT 5 *****/
/***** vvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef RAS_PICO_LAUTO
/*

  TE = 22
  TC = REN

  Data pin map
  ------------
  DIO1_PIN  10 : GPIB 10 : GPIO14
  DIO2_PIN  11 : GPIB 11 : GPIO15
  DIO3_PIN  12 : GPIB 12 : GPIO16
  DIO4_PIN  13 : GPIB 13 : GPIO17
  DIO5_PIN  14 : GPIB 14 : GPIO18
  DIO6_PIN  15 : GPIB 15 : GPIO19
  DIO7_PIN  20 : GPIB 20 : GPIO20
  DIO8_PIN  21 : GPIB 21 : GPIO21

  Control pin map
  ---------------
  IFC_PIN    9 : GPIB 9  : GPIO6  : b0
  NDAC_PIN   8 : GPIB 8  : GPIO7  : b1
  NRFD_PIN   7 : GPIB 7  : GPIO8  : b2
  DAV_PIN    6 : GPIB 6  : GPIO9  : b3
  EOI_PIN    5 : GPIB 5  : GPIO10 : b4
  REN_PIN    3 : GPIB 17 : GPIO11 : b5
  SRQ_PIN    4 : GPIB 10 : GPIO12 : b6
  ATN_PIN    2 : GPIB 11 : GPIO13 : b7

  Bits control lines as follows: 7-ATN_PIN, 6-SRQ_PIN, 5-REN_PIN, 4-EOI_PIN, 3-DAV_PIN, 2-NRFD_PIN, 1-NDAC_PIN, 0-IFC_PIN
    bits (databits) : State - 0=LOW, 1=HIGH/INPUT_PULLUP; Direction - 0=input, 1=output;
    mask (mask)     : 0=unaffected, 1=enabled
*/


uint8_t minPIN = 0xFF;
uint8_t maxPIN = 0;

const uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };
const uint8_t ctrlreg[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };

uint32_t gpioDbMask = 0;
uint32_t gpioCtrlMask = 0;


/***** Generate a GPIO mask from 8-bit mask *****/
uint32_t genGpioMask(const uint8_t buspins[], uint8_t bitmask){
  uint32_t gpiomask = 0;
  for (uint8_t i=0; i<8; i++) {
    if (bitmask & (1 << i)) gpiomask += (1 << buspins[i]);
  }
  return gpiomask;
}


/***** Calculate the minimum and maximum pin number *****/
void getMinMax(){
  uint8_t i = 0;
  for (i=0; i<8; i++) {
    if (databus[i] > maxPIN) maxPIN = databus[i];
    if (ctrlreg[i] > maxPIN) maxPIN = ctrlreg[i];
    if (databus[i] < minPIN) minPIN = databus[i];
    if (ctrlreg[i] < minPIN) minPIN = ctrlreg[i];   
  }
  
}

/*
void printMinMax(){
  Serial.print("Min: ");
  Serial.println(minPIN);
  Serial.print("Max: ");
  Serial.println(maxPIN);
}


void printPins(){
  bool state;
  for (uint8_t i=minPIN; i<=maxPIN; i++){
    state = digitalRead(i);
    Serial.print(F("Pin "));
    Serial.print(i);
    Serial.print(F(":\t"));
    Serial.println(state);
  }
}
*/

/***** Set GPIO internal pullup resistors *****/
void gpio_set_pullups_masked(uint32_t mask){
  const uint8_t min = minPIN;
  const uint8_t max = maxPIN + 1;
  for (uint8_t i=min; i<max; i++){
    if ( mask & (1<<i) ) gpio_pull_up(i);
  }
}


/***** Disable GPIO pull-up resistors *****/
void gpio_clear_pullups_masked(uint32_t mask){
  const uint8_t min = minPIN;
  const uint8_t max = maxPIN + 1;
  for (uint8_t i=min; i<max; i++){
    if ( mask & (1<<i) ) gpio_disable_pulls(i);
  }
}


/***** Initialise all GPIO pins *****/
void initRpGpioPins(){
  uint32_t gpiomask = 0;
  gpioDbMask = genGpioMask(databus, 0xFF);
  gpioCtrlMask = genGpioMask(ctrlreg, 0xFF);
  
  gpiomask = (gpioCtrlMask | gpioDbMask);     // Scope of GPIO pins to be allocated to GPIB
  getMinMax();                                // Get lowest and highest pin number
  gpio_init_mask(gpiomask);                   // Configure as GPIO pins those allocated to GPIB
  gpio_set_dir_in_masked(gpiomask);           // Configure all as inputs by default
  gpio_set_pullups_masked(gpiomask);          // Enable all pullups (un-asserted) by default
}


/***** Set the GPIB data bus to input pullup *****/
void readyGpibDbus(uint8_t state = INPUT_PULLUP) {

  // Set pins to OUTPUT
  if (state == OUTPUT) {
    // Set pins to output
    gpio_set_dir_out_masked(gpioDbMask);
    // Set outputs to high
    gpio_put_masked(gpioDbMask, gpioDbMask);
  }

  // Set data pins to input
  gpio_set_dir_in_masked(gpioDbMask);
  gpio_set_pullups_masked(gpioDbMask);
  
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {

  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  uint8_t result = 0;

  for (uint8_t i=0; i<8; i++) {
    if ( gpioall & (1<<databus[i]) ) result |= (1<<i);
  }

  return ~result;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {

  uint32_t gpiodb = 0;
  gpiodb = genGpioMask(databus, ~db);
  
  gpio_clear_pullups_masked(gpioDbMask);
  gpio_set_dir_out_masked(gpioDbMask);
  gpio_put_masked(gpioDbMask, gpiodb);

}


/***** Set the state of GPIB control pins *****/
void setGpibCtrlState(uint8_t bits, uint8_t mask){

  uint32_t gpiobits = genGpioMask(ctrlreg, bits);
  uint32_t gpiomask = genGpioMask(ctrlreg, mask);

  gpio_put_masked(gpiomask, gpiobits);
  
}


/***** Set the direction of GPIB control pins *****/
void setGpibCtrlDir(uint8_t bits, uint8_t mask){

  uint32_t gpioObits = genGpioMask(ctrlreg, (bits&mask));
  uint32_t gpioIbits = genGpioMask(ctrlreg, (~bits&mask));

  if (gpioObits){
    gpio_clear_pullups_masked(gpioObits);
    gpio_set_dir_out_masked(gpioObits);
  }

  if (gpioIbits){
    gpio_set_pullups_masked(gpioIbits);
    gpio_set_dir_in_masked(gpioIbits);
  }
  
}


/***** Get the state of a single pin *****/
uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // RAS_PICO_LAUTO
/***** ^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO BOARD LAYOUT 5 *****/
/***********************************/




/*******************************************/
/***** ADAFRUIT RP2040 ADLOGGER LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/
#ifdef ADAFRUIT_ADLOGGER


#endif // ADAFRUIT_ADLOGGER
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** ADAFRUIT RP2040 ADLOGGER LAYOUT *****/
/*******************************************/



/********************************************/
/***** NANO RP2040 CONNECT BOARD LAYOUT *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv *****/

/*
 * IN DEVELOPMENT
 */

#ifdef NANO_RP2040_CONNECT

const uint32_t gpioDbMask = 0x23F8000;
const uint32_t gpioCtrlMask = 0x300030D0;

uint32_t reverseBits(uint32_t dbyte) {
   dbyte = (dbyte & 0xF0) >> 4 | (dbyte & 0x0F) << 4;
   dbyte = (dbyte & 0xCC) >> 2 | (dbyte & 0x33) << 2;
   dbyte = (dbyte & 0xAA) >> 1 | (dbyte & 0x55) << 1;
   return dbyte;
}

void readyGpibDbus(uint8_t state = INPUT_PULLUP) {
  if (state == OUTPUT) {
  
    return;
  }
  // Set data pins to input  
  gpio_init_mask(0x023F8000);
  gpio_set_dir_in_masked(0x023F8000);
  gpio_set_pullups_masked(0x023F8000);
}


/***** Read the GPIB data bus wires to collect the byte of data *****/
uint8_t readGpibDbus() {
  // Read the byte of data on the bus
  uint32_t gpioall = gpio_get_all();
  uint32_t result1 = reverseBits(gpioall << 6);
  uint32_t result2 = gpioall >> 25;
  gpioall = result1 + result2;  
  return (uint8_t)~gpioall;
}


/***** Set the GPIB data bus to output and with the requested byte *****/
void setGpibDbus(uint8_t db) {
  uint32_t gpioall = db << 6;
  gpio_set_dir_out_masked(0x023F8000);
  gpio_put_masked(0x023F8000, gpioall);
}


uint8_t getGpibPinState(uint8_t pin){
  return digitalRead(pin);
}

#endif // NANO_RP2040_CONNECT
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** NANO RP2040 CONNECT BOARD LAYOUT *****/
/********************************************/



/*************************************/
/***** RAS PICO COMMON FUNCTIONS *****/
/***** vvvvvvvvvvvvvvvvvvvvvvvvv *****/
#if defined(RP2040_COMMON_FUNCTIONS)
void gpioFuncList(){
  /*
  Serial.print(F("Func: "));
  for (uint8_t i=6; i<21; i++){
    Serial.print(gpio_get_function(i));
  }
  */
  Serial.println();
  Serial.print(F("Pull: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_pulled_up(i));
  }
  Serial.println();
  Serial.print(F("Outs: "));
  for (uint8_t i=21; i>5; i--){
    Serial.print(gpio_is_dir_out(i));
  }
  Serial.println();
}
#endif  // RP2040_COMMON_FUNCTIONS
/***** ^^^^^^^^^^^^^^^^^^^^^^^^^ *****/
/***** RAS PICO COMMON FUNCTIONS *****/
/*************************************/

//  "connections": [ [ "pico:GP0", "$serialMonitor:RX", "" ], [ "pico:GP1", "$serialMonitor:TX", "" ] ],

