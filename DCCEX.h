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

#if __has_include ( "myTPL.h")
  #include "TPL.h"
  #include "TPLMacros.h"
  #include "TPLLayout.h"
  #include "myTPL.h"
  void DCC::beginTPL() {TPL::begin();}
  void DCC::loopTPL() {TPL::loop();}
  LayoutManager * LayoutManager::manager=new TPLLayout();
#else 
  void DCC::beginTPL(){};
  void DCC::loopTPL(){};
  LayoutManager * LayoutManager::manager=new JMRILayout();
#endif

#endif
