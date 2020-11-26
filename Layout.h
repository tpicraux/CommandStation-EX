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
 #ifndef Layout_H
 #define Layout_H
#include "Layout.h" 
#include <Adafruit_MCP23017.h>


// Type masks for searching by type without regard to technology 
const byte         LAYOUT_TYPE_MASK=0xF0;   // allows search on turnout/sensor/output/signal
const byte         LAYOUT_TYPE_TURNOUT=0x10;   // allows search on turnout/sensor/output/signal
const byte         LAYOUT_TYPE_SENSOR=0x20;   // allows search on turnout/sensor/output/signal
const byte         LAYOUT_TYPE_OUTPUT=0x30;   // allows search on turnout/sensor/output/signal
const byte         LAYOUT_TYPE_SIGNAL=0x40;   // allows search on turnout/sensor/output/signal

// Unique layout items 
const byte LAYOUT_SERVO_TURNOUT=LAYOUT_TYPE_TURNOUT | 0x01;
const byte LAYOUT_DCC_TURNOUT=LAYOUT_TYPE_TURNOUT | 0x02;
const byte LAYOUT_PIN_TURNOUT=LAYOUT_TYPE_TURNOUT | 0x03;
const byte LAYOUT_I2CPIN_TURNOUT=LAYOUT_TYPE_TURNOUT | 0x04;

const byte LAYOUT_I2C_SENSOR=LAYOUT_TYPE_SENSOR | 0x01;
const byte LAYOUT_PIN_SENSOR=LAYOUT_TYPE_SENSOR | 0x02;

const byte LAYOUT_I2C_OUTPUT=LAYOUT_TYPE_OUTPUT | 0x01;
const byte LAYOUT_PIN_OUTPUT=LAYOUT_TYPE_OUTPUT | 0x02;

const byte LAYOUT_I2C_SIGNAL=LAYOUT_TYPE_SIGNAL | 0x01;
const byte LAYOUT_PIN_SIGNAL=LAYOUT_TYPE_SIGNAL | 0x02;

const byte LAYOUT_SLOT_WIDTH=7;
 
  // Flag bits for status of hardware and TPL
  static const short SECTION_FLAG = 0x01;
  static const short SENSOR_FLAG = 0x02;
  static const short SIGNAL_FLAG_RED = 0x04;
  static const short SIGNAL_FLAG_GREEN = 0x08; 
  static const short SIGNAL_FLAG_AMBER = SIGNAL_FLAG_RED | SIGNAL_FLAG_GREEN;
  static const short TURNOUT_FLAG_LEFT = 0x10;
  static const short TURNOUT_FLAG_RIGHT = 0x20;

#if (defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_SAMD_ZERO))
   static const short MAX_FLAGS=256;
   #define FLAGOVERFLOW(x) false
#else
  static const short MAX_FLAGS=64;
  #define FLAGOVERFLOW(x) x>=MAX_FLAGS
#endif

class Layout  {
  public:
       static void begin();
       static bool setTurnout(byte id, bool left);
       static int getSensor(byte id);
       static bool setOutput(byte id, bool on);
       static bool setSignal(byte id, char RGA);
       static bool defineTurnout(int id, int addr, byte subaddr);
       static bool deleteTurnout(int id);
       static bool streamTurnoutList(Print * stream, bool withrottleStyle); // or JMRI style if false
       
     // Part 2 functions associated with TPL 
       static void streamFlags(Print* stream);
       static void setFlag(byte id,byte onMask, byte OffMask=0);
       static byte getFlag(byte id,byte mask);    

       static const  PROGMEM  byte PredefinedLayout[];
       static const int EEPROM_FLAGS_START=32; 
       static const int EEPROM_LAYOUT_START=EEPROM_FLAGS_START+MAX_FLAGS;
      
  private:
     static void getSlot(int slotno);
     static int getSlot(byte type, byte id);
     static void pinsetup(int ponPos, byte direction);
     static void i2cpinsetup(byte, byte direction);
     static bool predefinedLayout;   
     static Adafruit_MCP23017 * mcp[4]; 
     static byte flags[MAX_FLAGS];
     static byte slot[LAYOUT_SLOT_WIDTH];  // TODO... make a struct union over this

      
};

#endif
