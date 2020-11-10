#include <Arduino.h>
#include "TPLLayout.h"
#include <DIO2.h>
#include "DIAG.h"

void TPLLayout::begin() {
  // TODO presets and pins and eeprom
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
         int angle=pgm_read_word_near(Layout+slot+3 + left?0:2);
         // TODO set serveo turnout (pin,angle)
        }
        break;
    
    case LAYOUT_DCC:  // tech, id, addr, subaddr,leftisActive
        {
          int addr=pgm_read_word_near(Layout+slot+2);
          byte subaddr=pgm_read_byte_near(Layout+slot+4);
          byte leftIsActive=pgm_read_byte_near(Layout+slot+5);
          // TODO send DCC turnout addr,subaddr, leftIsActive?left:!left);
        }
        break;
         
    case LAYOUT_I2CPIN:  // tech, id, leftpin, rightpin
        {
          byte pin=pgm_read_byte_near(Layout+slot+left?2:3);
          // TODO setI2cPin(pin);
        }
        break;
        
    case LAYOUT_PIN:  // tech, id, leftpin, rightpin
        {
          byte pin=pgm_read_byte_near(Layout+slot+left?2:3);
          // TODO digitalWrite(pin,LOW);
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
     case LAYOUT_I2CPIN:
          return  false; // TODO i2csensor(pin) ^ invert;
     case LAYOUT_PIN:
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
     case LAYOUT_I2CPIN:
          // TODO seti2cpin(pin, on ^ invert;
          break;
     case LAYOUT_PIN:
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
