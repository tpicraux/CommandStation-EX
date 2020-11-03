// This include is intended to visually simplify the .ino for the end users.
// If there were any #ifdefs required they are much better handled in here.  

#ifndef DCCEX_h
#define DCCEX_h
#include <Arduino.h>
#include "defines.h"
#include "DCC.h"
#include "DIAG.h"
#include "DCCEXParser.h"
#include "version.h"
#include "WifiInterface.h"
#include "EthernetInterface.h"
#include "LCD_Implementation.h"
#include "freeMemory.h"
#include "TPL.h"

#if __has_include ( "myRoutes.h")
  #include "myRoutes.h"
  #include "TPL2.h"
  void TPL::begin() {TPL2::begin();}
  void TPL::loop() {TPL2::loop();}
#else 
  void TPL::begin(){};
  void TPL::loop(){};
#endif

#endif
