 #ifndef TPLLayout_H
 #define TPLLayout_H
 
 enum LAYOUTCODE {
         LAYOUT_TYPE_MASK=0xF0,
         LAYOUT_TURNOUT=0x10,
         LAYOUT_SENSOR=0x20,
         LAYOUT_OUTPUT=0x30,
         LAYOUT_SIGNAL=0x40,
         LAYOUT_TECHNOLOGY_MASK=0x0F,
         LAYOUT_I2CSERVO=0x00,
         LAYOUT_DCC=0x01,
         LAYOUT_PIN=0x02,
         LAYOUT_I2CPIN=0x03,
         LAYOUT_SLOT_WIDTH=6
 };

class TPLLayout {
  public:
     static void begin();
     static bool setTurnout(byte id, bool left);
     static int getSensor(byte id);
     static bool setOutput(byte id, bool on);
     static bool setSignal(byte id, bool red);
     static const  PROGMEM  byte Layout[]; 
  private:
     static int getSlot(byte type, byte id);   
};
#endif
