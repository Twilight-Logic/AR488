#include "Custom.h"


/******************************/
/***** CUSTOM PIN LAYOUT  *****/
/***** vvvvvvvvvvvvvvvvvv *****/
#ifdef AR488_CUSTOM_LAYOUT


uint8_t databus[8] = { DIO1_PIN, DIO2_PIN, DIO3_PIN, DIO4_PIN, DIO5_PIN, DIO6_PIN, DIO7_PIN, DIO8_PIN };

uint8_t ctrlbus[8] = { IFC_PIN, NDAC_PIN, NRFD_PIN, DAV_PIN, EOI_PIN, REN_PIN, SRQ_PIN, ATN_PIN };


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


#endif
/***** ^^^^^^^^^^^^^^^^^ *****/
/***** CUSTOM PIN LAYOUT *****/
/*****************************/
