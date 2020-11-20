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
#include "TPLLayout.h"
#include <DIO2.h>
#include "PWMservoDriver.h"
#include "DIAG.h"
#include "DCC.h"
Adafruit_MCP23017 * TPLLayout::mcp[4] = {NULL, NULL, NULL, NULL};

void TPLLayout::begin()  {
  // presets and pins and eeprom
   for (int slot=0;;slot+=LAYOUT_SLOT_WIDTH) {
      byte tech=pgm_read_byte_near(Layout+slot);
      DIAG(F("\nLayout %x"),tech);
      if (!tech) break;
      switch (tech) {
        case LAYOUT_SERVO_TURNOUT:
        case  LAYOUT_DCC_TURNOUT:
             break;
        
        case LAYOUT_PIN_TURNOUT:
        case LAYOUT_PIN_OUTPUT:
             pinsetup(slot+2,OUTPUT);
             break; 
 
        case LAYOUT_I2CPIN_TURNOUT:
        case LAYOUT_I2C_OUTPUT:
             i2cpinsetup(slot+2,OUTPUT);
             break;

        case LAYOUT_I2C_SENSOR:
             i2cpinsetup(slot+2,INPUT);
             break;

       case  LAYOUT_PIN_SENSOR:
             pinsetup(slot+2,INPUT);
             break;

       case LAYOUT_I2C_SIGNAL:
             i2cpinsetup(slot+2,OUTPUT);
             i2cpinsetup(slot+3,OUTPUT);
             i2cpinsetup(slot+4,OUTPUT);
             break; 
             
       case LAYOUT_PIN_SIGNAL:
             pinsetup(slot+2,OUTPUT);
             pinsetup(slot+3,OUTPUT);
             pinsetup(slot+4,OUTPUT);
             break; 
      }
   }
}

void TPLLayout::i2cpinsetup(int pinpos, byte direction) {
  byte pin=pgm_read_byte_near(Layout+pinpos);
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

void TPLLayout::pinsetup(int pinpos, byte direction) {
  byte pin=pgm_read_byte_near(Layout+pinpos);
  pinMode(pin,direction);
}
   
bool TPLLayout::setTurnout(byte id, bool left) {
  int slot=getSlot(LAYOUT_TYPE_TURNOUT,id);
  if (slot<0) {
          DIAG(F("\nTPL missing turnout %d\n"),id);
          return false;
          }
          
   switch (pgm_read_byte_near(Layout+slot)) {
    case LAYOUT_SERVO_TURNOUT:  //tech, id, pin, leftAngle, rightAngle
        {
         byte pin=pgm_read_byte_near(Layout+slot+2);
         uint16_t angle=pgm_read_word_near(Layout+slot+ (left?3:5));
         PWMServoDriver::setServo(pin,angle);
        }
        break;
    
    case LAYOUT_DCC_TURNOUT:  // tech, id, addr, subaddr,leftisActive
        {
          int addr=pgm_read_word_near(Layout+slot+2);
          byte subaddr=pgm_read_byte_near(Layout+slot+4);
          byte leftIsActive=pgm_read_byte_near(Layout+slot+5);
          DCC::setAccessory(addr,subaddr, leftIsActive?left:!left);
        }
        break;
         
    case LAYOUT_I2CPIN_TURNOUT:  // tech, id, pin, leftValue
        {
          byte pin=pgm_read_byte_near(Layout+slot+2);
          byte leftValue=pgm_read_byte_near(Layout+slot+3);
          if (!left) leftValue=~leftValue; 
          mcp[pin / 16]->digitalWrite(pin % 16,leftValue);          
        }
        break;
        
    case LAYOUT_PIN_OUTPUT:  // tech, id, pin, leftValue
        {
          byte pin=pgm_read_byte_near(Layout+slot+2);
          byte leftValue=pgm_read_byte_near(Layout+slot+3);
          if (!left) leftValue=~leftValue; 
          digitalWrite2(pin,leftValue);
        }
        break;

    default:
      break;              
   }
   return true;
}

int TPLLayout::getSensor(byte id) {
  int slot=getSlot(LAYOUT_TYPE_SENSOR,id);
  if (slot<0) return -1; // missing sensors are just virtual
          
   byte tech=pgm_read_byte_near(Layout+slot+1);
   byte pin=pgm_read_byte_near(Layout+slot+2);
   bool invert=pgm_read_byte_near(Layout+slot+3);
   switch (tech) {
     case LAYOUT_I2C_SENSOR:
          return  mcp[pin / 16]->digitalRead(pin % 16) ^ invert;
     case LAYOUT_PIN_SENSOR:
          return  (digitalRead2(pin)==LOW) ^ invert;
     default: 
          return -1;
   }              
}

bool TPLLayout::setOutput(byte id, bool on) {
    int slot=getSlot(LAYOUT_TYPE_OUTPUT,id);
    if (slot<0) return false; // missing outputs are just virtual
          
   byte tech=pgm_read_byte_near(Layout+slot+1);
   byte pin=pgm_read_byte_near(Layout+slot+2);
   bool invert=pgm_read_byte_near(Layout+slot+3);
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


bool TPLLayout::setSignal(byte id, char rga){
  int slot=getSlot(LAYOUT_TYPE_OUTPUT,id);
  if (slot<0) return false; // missing outputs are just virtual
   byte tech=pgm_read_byte_near(Layout+slot+1);
   byte pin[3];
   pin[0]=pgm_read_byte_near(Layout+slot+2);
   pin[1]=pgm_read_byte_near(Layout+slot+3);
   pin[2]=pgm_read_byte_near(Layout+slot+4);
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
  return true;
}

bool TPLLayout::streamTurnoutList(Print * stream, bool withrottleStyle) {
   bool foundSome=false;
   for (int slot=0;;slot+=LAYOUT_SLOT_WIDTH) {
      byte b=pgm_read_byte_near(Layout+slot);
      if (!b) break;
      if ((b & LAYOUT_TYPE_MASK)==LAYOUT_TYPE_TURNOUT) {
        foundSome=true;
        byte id=pgm_read_byte_near(Layout+slot+1);
        // TODO get turnout status from flags
        if (withrottleStyle) StringFormatter::send(stream,F("]\\[%d}|{%d}|{2"), id, id);
        else StringFormatter::send(stream,F("<H %d 0>"), id);
      }
   }
   return foundSome;
}

int TPLLayout::getSlot(byte type,byte id) {
    for (int slot=0;;slot+=LAYOUT_SLOT_WIDTH) {
      byte b=pgm_read_byte_near(Layout+slot);
      if (!b) break;
      if ((b & LAYOUT_TYPE_MASK)==type) {
        b=pgm_read_byte_near(Layout+slot+1);
        if (b==id) return slot;
      }
    }
    return -1;
}


bool TPLLayout::defineTurnout(int id, int addr, byte subaddr) {return false;}
bool TPLLayout::deleteTurnout(int id){return false;}
