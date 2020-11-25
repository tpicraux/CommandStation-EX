/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  © 2020, Harald Barth.
 *  
 *  This file is part of CommandStation-EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */
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


// 3 Possible combinations
// 1- JMRI standard layout control (alllows JMRI to create turnouts/sensors etc)
// 2- TPL standard layout control (myLayout.h defines turnouts/sensors etc)
// 3- TPL standard layout control with automation (myLayout.h and myTPL.h contains automation)
#include "LayoutManager.h"

#if __has_include ( "myLayout.h")
  #include "TPL.h"
  #include "TPLMacros.h"
  #include "TPLLayout.h"
  #include "myLayout.h"
  LayoutManager * LayoutManager::manager=new TPLLayout();
  #if __has_include ( "myTPL.h")
    #include "myTPL.h"
    void DCC::beginTPL() {LayoutManager::manager->begin(); TPL::begin();}
    void DCC::loopTPL() {TPL::loop();}
  #else
    void DCC::beginTPL() {LayoutManager::manager->begin();}
    void DCC::loopTPL() {}  
  #endif      
#else 
  #include "JMRILayout.h"
  LayoutManager * LayoutManager::manager=new JMRILayout();
  void DCC::beginTPL(){LayoutManager::manager->begin();};
  void DCC::loopTPL(){};
#endif

#endif
