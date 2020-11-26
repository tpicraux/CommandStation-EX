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
#include "Layout.h"
#include <DIO2.h>
#include "PWMservoDriver.h"
#include "DIAG.h"
#include "DCC.h"
#include "EEPROM.h"
Adafruit_MCP23017 * Layout::mcp[4] = {NULL, NULL, NULL, NULL};

#define SLOTWORD(x)  ( slot[x]+(slot[x+1]<<8))     

byte Layout::flags[MAX_FLAGS];
byte Layout::slot[LAYOUT_SLOT_WIDTH];
bool Layout::predefinedLayout=false;

void Layout::begin()  {
  predefinedLayout=pgm_read_byte_near(PredefinedLayout)!=0xFF;
  // TODO check EEPROM stamp 
  EEPROM.get(EEPROM_FLAGS_START,flags);
  // presets and pins and eeprom
   for (int slotno=0;;slotno++) {
      getSlot(slotno);
      if (!slot[0]) break;
      byte tech=slot[0];
      DIAG(F("\nLayout %x"),tech);
      switch (tech) {
        case LAYOUT_SERVO_TURNOUT:
        case  LAYOUT_DCC_TURNOUT:
             break;
        
        case LAYOUT_PIN_TURNOUT:
        case LAYOUT_PIN_OUTPUT:
             pinMode(slot[2],OUTPUT);
             break; 
 
        case LAYOUT_I2CPIN_TURNOUT:
        case LAYOUT_I2C_OUTPUT:
             i2cpinsetup(slot[2],OUTPUT);
             break;

        case LAYOUT_I2C_SENSOR:
             i2cpinsetup(slot[2],INPUT);
             break;

       case  LAYOUT_PIN_SENSOR:
             pinMode(slot[2],INPUT_PULLUP);
             break;

       case LAYOUT_I2C_SIGNAL:
             i2cpinsetup(slot[2],OUTPUT);
             i2cpinsetup(slot[3],OUTPUT);
             i2cpinsetup(slot[4],OUTPUT);
             break; 
             
       case LAYOUT_PIN_SIGNAL:
             pinMode(slot[2],OUTPUT);
             pinMode(slot[3],OUTPUT);
             pinMode(slot[4],OUTPUT);
             break; 
      }
   }
}

void Layout::i2cpinsetup(byte pin, byte direction) {
  byte board = pin / 16;
  byte subPin = pin % 16;
  if (!mcp[board]) {
     mcp[board] = new  Adafruit_MCP23017();
     mcp[board]->begin(board);      // use default address 0
  }
  if (direction==INPUT) {
     mcp[board]->pinMode(subPin, INPUT);
     mcp[board]->pullUp(subPin, HIGH);
  }
  else  mcp[board]->pinMode(subPin, OUTPUT);                                 
}
   
bool Layout::setTurnout(byte id, bool left) {
  if (getSlot(LAYOUT_TYPE_TURNOUT,id)<0) {  
        DIAG(F("\nLayout missing turnout %d\n"),id);
        return false;
     }
          
   switch (slot[0]) {
    case LAYOUT_SERVO_TURNOUT:  //tech, id, pin, leftAngle, rightAngle
        {
         byte pin=slot[2];
         uint16_t angle;
         if (left) angle=SLOTWORD(3);
            else   angle=SLOTWORD(5);
         PWMServoDriver::setServo(pin,angle);
        }
        break;
    case LAYOUT_DCC_TURNOUT:  // tech, id, addr, subaddr,leftisActive
        {
          int addr=SLOTWORD(2);
          byte subaddr=slot[4];
          byte leftIsActive=slot[5];
          DCC::setAccessory(addr,subaddr, leftIsActive?left:!left);
        }
        break;
         
    case LAYOUT_I2CPIN_TURNOUT:  // tech, id, pin, leftValue
        {
          byte pin=slot[2];
          byte leftValue=slot[3];
          if (!left) leftValue=~leftValue; 
          mcp[pin / 16]->digitalWrite(pin % 16,leftValue);          
        }
        break;
        
    case LAYOUT_PIN_OUTPUT:  // tech, id, pin, leftValue
        {
          byte pin=slot[2];
          byte leftValue=slot[3];
          if (!left) leftValue=~leftValue; 
          digitalWrite2(pin,leftValue);
        }
        break;

    default:
      break;              
   }
   setFlag(id,left?TURNOUT_FLAG_LEFT:TURNOUT_FLAG_RIGHT, TURNOUT_FLAG_LEFT|TURNOUT_FLAG_RIGHT);
   return true;
}

int Layout::getSensor(byte id) {
  if (flags[id] & SENSOR_FLAG) return 1;  // sensor set on in code         
  if (getSlot(LAYOUT_TYPE_SENSOR,id)<0) return -1; // missing sensors are just virtual
   byte tech=slot[1];
   byte pin=slot[2];
   bool invert=slot[3];
   switch (tech) {
     case LAYOUT_I2C_SENSOR:
          return  mcp[pin / 16]->digitalRead(pin % 16) ^ invert;
     case LAYOUT_PIN_SENSOR:
          return  (digitalRead2(pin)==LOW) ^ invert;
     default: 
          return -1;
   }              
}

bool Layout::setOutput(byte id, bool on) {
    if (getSlot(LAYOUT_TYPE_OUTPUT,id)<0) return false; // missing outputs are just virtual
          
   byte tech=slot[1];
   byte pin=slot[2];
   bool invert=slot[3];
   switch (tech) {
     case LAYOUT_I2C_OUTPUT:
          mcp[pin / 16]->digitalWrite(pin % 16,on ^ invert);
          break;
     case LAYOUT_PIN_OUTPUT:
          digitalWrite2(pin, (on ^ invert)?LOW:HIGH);
          break;
     default: 
          return false;
   }              
   return true;
}


bool Layout::setSignal(byte id, char rga){
  if (getSlot(LAYOUT_TYPE_OUTPUT,id)<0) return false; // missing outputs are just virtual
   byte tech=slot[1];
   byte pin[3];
   pin[0]=slot[2];
   pin[1]=slot[3];
   pin[2]=slot[4];
   switch (tech) {
      case LAYOUT_I2C_SIGNAL:
           mcp[pin[0] / 16]->digitalWrite(pin[0] % 16,rga=='R'?LOW:HIGH);          
           mcp[pin[1] / 16]->digitalWrite(pin[1] % 16,rga=='G'?LOW:HIGH);          
           mcp[pin[2] / 16]->digitalWrite(pin[2] % 16,rga=='A'?LOW:HIGH);          
           break;
      case LAYOUT_PIN_SIGNAL:
           digitalWrite2(pin[0],rga=='R'?LOW:HIGH);          
           digitalWrite2(pin[1],rga=='G'?LOW:HIGH);          
           digitalWrite2(pin[2],rga=='A'?LOW:HIGH);          
           break;
   } 
   switch (rga) {
     case 'R': 
         setFlag(id,SIGNAL_FLAG_RED,SIGNAL_FLAG_GREEN);
         break;
     case 'G': 
         setFlag(id,SIGNAL_FLAG_GREEN,SIGNAL_FLAG_RED);
         break;
     case 'A': 
         setFlag(id,SIGNAL_FLAG_AMBER);
         break;
   }
      
  return true;
}

bool Layout::streamTurnoutList(Print * stream, bool withrottleStyle) {
   bool foundSome=false;
   for (int slotno=0;;slotno++) {
      getSlot(slotno);
      if (!slot[0]) break;
      if ((slot[0] & LAYOUT_TYPE_MASK)==LAYOUT_TYPE_TURNOUT) {
        foundSome=true;
        byte id=slot[1];
        // TODO get turnout status from flags
        if (withrottleStyle) StringFormatter::send(stream,F("]\\[%d}|{%d}|{2"), id, id);
        else StringFormatter::send(stream,F("<H %d 0>"), id);
      }
   }
   return foundSome;
}


void Layout::getSlot(int slotno) {
    if (predefinedLayout) {
        for (int i=0;i<LAYOUT_SLOT_WIDTH;i++) slot[i]=pgm_read_byte_near(PredefinedLayout+(LAYOUT_SLOT_WIDTH*slotno)+i);
    }
    else {
        for (int i=0;i<LAYOUT_SLOT_WIDTH;i++) slot[i]=EEPROM.read(EEPROM_LAYOUT_START+(LAYOUT_SLOT_WIDTH*slotno)+i);    
    }
}

int Layout::getSlot(byte type,byte id) {
     for (int slotno=0;;slotno++) {
        getSlot(slotno);
        if (!slot[0]) return -1;  // end of slot table 
        if (slot[1]==id && ((slot[0] & LAYOUT_TYPE_MASK)==type)) return slotno;           
     }
}

void Layout::setFlag(byte id,byte onMask, byte offMask) {
  
   if (FLAGOVERFLOW(id)) return; // Outside UNO range limit
   byte f=flags[id];
   byte previous=f;
   f &= ~offMask;
   f |= onMask;
   if (f!=flags[id]) {
       flags[id]=f;
       if (f!=previous) EEPROM.write(EEPROM_FLAGS_START+id, f);
   }
}
byte Layout::getFlag(byte id,byte mask) {
   if (FLAGOVERFLOW(id)) return 0; // Outside UNO range limit
   return flags[id]&mask;   
}


bool Layout::defineTurnout(int id, int addr, byte subaddr) {
  if (predefinedLayout || (getSlot(LAYOUT_TYPE_TURNOUT,id)>=0)) return false; 
  
  // TODO... create slot on EEPROM ... find an empty slot 
  return false;
}
  
bool Layout::deleteTurnout(int id) {
  if (predefinedLayout) return false;
  int sno= getSlot(LAYOUT_TYPE_TURNOUT,id);
  if (sno>=0) {
    EEPROM.write(EEPROM_LAYOUT_START+sno*LAYOUT_SLOT_WIDTH, 0xff); // mark unused  (0=end of list!)
    EEPROM.write(EEPROM_LAYOUT_START+sno*LAYOUT_SLOT_WIDTH+1, 0x00); // id 0 
  } 
  return true;
}

 void Layout::streamFlags(Print* stream) {                
     for (int id=0;id<MAX_FLAGS; id++) {
     byte flag=flags[id];
     if (flag) {
       StringFormatter::send(stream,F("\nflags[%d} "),id);
       if (flag & SECTION_FLAG) StringFormatter::send(stream,F(" RESERVED"));
       if (flag & TURNOUT_FLAG_LEFT) StringFormatter::send(stream,F(" TL"));
       if (flag & TURNOUT_FLAG_RIGHT) StringFormatter::send(stream,F(" TR"));
       if (flag & SENSOR_FLAG) StringFormatter::send(stream,F(" SET"));
       if ((flag & SIGNAL_FLAG_AMBER) ==SIGNAL_FLAG_AMBER) 
            StringFormatter::send(stream,F(" AMBER"));
       else if (flag & SIGNAL_FLAG_RED) 
            StringFormatter::send(stream,F(" RED"));
       else if (flag & SIGNAL_FLAG_GREEN) 
            StringFormatter::send(stream,F(" GREEN"));
     }                 
   }
   StringFormatter::send(stream,F("\n"));
}
