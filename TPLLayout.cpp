#include <Arduino.h>
#include "TPLLayout.h"
#include <DIO2.h>
#include "PWMservoDriver.h"
#include "DIAG.h"
#include "DCC.h"

Adafruit_MCP23017 * TPLLayout::mcp[4] = {NULL, NULL, NULL, NULL};

void TPLLayout::begin() {
  // presets and pins and eeprom
   for (int slot=0;;slot+=LAYOUT_SLOT_WIDTH) {
      byte tech=pgm_read_byte_near(Layout+slot) & LAYOUT_TECHNOLOGY_MASK;
      if (!tech) break;
      switch (tech) {
        case LAYOUT_I2CPIN_IN:
        case LAYOUT_I2CPIN_OUT:
             {
              byte pin=pgm_read_byte_near(Layout+slot+2);
              byte board = pin / 16;
              byte subPin = pin % 16;
              if (!mcp[board]) {
                  mcp[board] = new  Adafruit_MCP23017();
                  mcp[board]->begin(board);      // use default address 0
              }
              if (tech==LAYOUT_I2CPIN_IN) {
                mcp[board]->pinMode(subPin, INPUT);
                mcp[board]->pullUp(subPin, HIGH);
              }
              else  mcp[board]->pinMode(subPin, OUTPUT);                                 
             }
             break;
        case LAYOUT_PIN_IN:
             {
              byte pin=pgm_read_byte_near(Layout+slot+2);
              pinMode(pin,INPUT_PULLUP);
             }
             break;
        case LAYOUT_PIN_OUT:
             {
              byte pin=pgm_read_byte_near(Layout+slot+2);
              pinMode(pin,OUTPUT);
             }
             break;
        
        case LAYOUT_I2CSERVO:
             // TODO
        default: break;
      }
   }
      
}
   
bool TPLLayout::setTurnout(byte id, bool left) {
  int slot=getSlot(LAYOUT_TURNOUT,id);
  if (slot<0) {
          DIAG(F("\nTPL missing turnout %d\n"),id);
          return false;
          }
          
   byte tech=pgm_read_byte_near(Layout+slot+1) & LAYOUT_TECHNOLOGY_MASK;
   switch (tech) {
    case LAYOUT_I2CSERVO:  //tech, id, pin, leftAngle, rightAngle
        {
         byte pin=pgm_read_byte_near(Layout+slot+2);
         uint16_t angle=pgm_read_word_near(Layout+slot+3 + left?0:2);
         PWMServoDriver::setServo(pin,angle);
        }
        break;
    
    case LAYOUT_DCC:  // tech, id, addr, subaddr,leftisActive
        {
          int addr=pgm_read_word_near(Layout+slot+2);
          byte subaddr=pgm_read_byte_near(Layout+slot+4);
          byte leftIsActive=pgm_read_byte_near(Layout+slot+5);
          DCC::setAccessory(addr,subaddr, leftIsActive?left:!left);
        }
        break;
         
    case LAYOUT_I2CPIN_OUT:  // tech, id, leftpin, rightpin
        {
          byte pin=pgm_read_byte_near(Layout+slot+left?2:3);
          mcp[pin / 16]->digitalWrite(pin % 16,LOW);          
        }
        break;
        
    case LAYOUT_PIN_OUT:  // tech, id, leftpin, rightpin
        {
          byte pin=pgm_read_byte_near(Layout+slot+left?2:3);
          digitalWrite2(pin,LOW);
        }
        break;

    default:
      break;              
   }
   return true;
}

int TPLLayout::getSensor(byte id) {
  int slot=getSlot(LAYOUT_SENSOR,id);
  if (slot<0) return -1; // missing sensors are just virtual
          
   byte tech=pgm_read_byte_near(Layout+slot+1) & LAYOUT_TECHNOLOGY_MASK;
   byte pin=pgm_read_byte_near(Layout+slot+2);
   bool invert=pgm_read_byte_near(Layout+slot+3);
   switch (tech) {
     case LAYOUT_I2CPIN_IN:
          return  mcp[pin / 16]->digitalRead(pin % 16) ^ invert;
     case LAYOUT_PIN_IN:
          return  (digitalRead2(pin)==LOW) ^ invert;
     default: 
          return -1;
   }              
}

bool TPLLayout::setOutput(byte id, bool on) {
    int slot=getSlot(LAYOUT_OUTPUT,id);
    if (slot<0) return false; // missing outputs are just virtual
          
   byte tech=pgm_read_byte_near(Layout+slot+1) & LAYOUT_TECHNOLOGY_MASK;
   byte pin=pgm_read_byte_near(Layout+slot+2);
   bool invert=pgm_read_byte_near(Layout+slot+3);
   switch (tech) {
     case LAYOUT_I2CPIN_OUT:
          mcp[pin / 16]->digitalWrite(pin % 16,on ^ invert);
          break;
     case LAYOUT_PIN_OUT:
          digitalWrite2(pin, (on ^ invert)?LOW:HIGH);
          break;
     default: 
          return false;
   }              
   return true;
}


bool TPLLayout::setSignal(byte id, bool red){
  // TODO
  (void)id;
  (void)red; 
  return true;
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
