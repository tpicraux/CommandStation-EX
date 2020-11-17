 #ifndef TPLLayout_H
 #define TPLLayout_H
 
#include <Adafruit_MCP23017.h>


// Type masks for searching by type withoyr regard to technology 
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
 

class TPLLayout {
  public:
     static void begin();
     static bool setTurnout(byte id, bool left);
     static int getSensor(byte id);
     static bool setOutput(byte id, bool on);
     static bool setSignal(byte id, char RGA);
     static void streamTurnoutList(Print * stream);
     static const  PROGMEM  byte Layout[]; 
  private:
     static int getSlot(byte type, byte id);
     static void pinsetup(int ponPos, byte direction);
     static void i2cpinsetup(int ponPos, byte direction);
     
     static Adafruit_MCP23017 * mcp[4]; 
};
#endif
