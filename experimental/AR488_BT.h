#ifndef AR488_BT_H
#define AR488_BT_H

#include <Arduino.h>
#include "AR488_Config.h"

#ifdef AR_BT_EN


  /***** AR488_BT.h, ver. 0.48.24, 25/04/2020 *****/
  /*
  * AR488 HC05 BlueTooth module headers
  */

  void btInit();
  void blinkLed(uint8_t count);
  bool btChkCfg();
  bool btCfg();
  bool detectBaud();
  bool atReply(const char* reply);

#endif  // AR_BT_EN

#endif // AR488_BT_H
