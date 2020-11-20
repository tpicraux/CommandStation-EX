/*
 *  © 2020, Chris Harlow. All rights reserved.
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
#include <Arduino.h>
#include "JMRILayout.h"
#include <DIO2.h>
#include "PWMservoDriver.h"
#include "Turnouts.h"
#include "DIAG.h"
#include "DCC.h"

void JMRILayout::begin()  {
   //  TODO load status from  EPROM 

}

bool JMRILayout::defineTurnout(int id, int addr, byte subaddr) {
         return Turnout::create(id,addr,subaddr);
}
bool JMRILayout::deleteTurnout(int id) {
         return Turnout::remove(id);
}

bool JMRILayout::setTurnout(byte id, bool left) {
   Turnout *tt = Turnout::get(id);
   if (!tt) return false;
   tt->activate(id,left);
   return true;
}

int JMRILayout::getSensor(byte id) {
    //TODO 
    return -1; // missing sensors are just virtual
}      
 

bool JMRILayout::setOutput(byte id, bool on) {
  return false;  // TODO
 }


bool JMRILayout::setSignal(byte id, char rga){
  return false;  // Not supported
}

bool JMRILayout::streamTurnoutList(Print * stream, bool withrottleStyle) {
   bool foundSome=false;
   for (Turnout *tt = Turnout::firstTurnout; tt != NULL; tt = tt->nextTurnout) {
      foundSome = true;
      int id=tt->data.id;      
      if (withrottleStyle) StringFormatter::send(stream,F("]\\[%d}|{%d}|{2"), id, id);
      else StringFormatter::send(stream, F("<H %d %d>"), id, (tt->data.tStatus & STATUS_ACTIVE)!=0);       
   }
   return foundSome;
}
