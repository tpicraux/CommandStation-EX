   // Describe the turnouts, signals and sensors
   //       TPL refers to turnouts by id and switches them left or right to avoid ambiguity.
    
   //   I2CTURNOUT(byte id, byte pin, int servoLeft, int servoRight);
   //   PINTURNOUT(byte id, byte pinleft, bool leftIsLOW)
   //   DCCTURNOUT(byte id, int dccAddress, byte subAddress, bool activatedIsLeft);
 
   // I2C turnouts I2CTURNOUT(byte id, byte pin, int servoLeft, int servoRight)
   //               id  = unique id (0-255)  used in TL() and TR() and JMRI (if used)
   //               pin   = pin position relative to first PWM servo board (second board starts at 16 etc)
   //               leftAngle  = servo position for turn left    
   //               rightAngle = servo position for turn right
   //   
   // DCC turnouts DCCTURNOUT(byte id, int dccAddress, byte subAddress, bool activatedIsLeft)
   //               id = unique id (0-64) used in TL() and TR() and JMRI (if used)
   //               dccAddress = DCC accessory decoder address
   //               subAddress  = accesory sub address    
   //               activatedIsLeft = true if point thrown means left exit.
 
 LAYOUT
   I2CTURNOUT(0, 0, 150, 195);       
   I2CTURNOUT(1, 1, 150, 195);       
   I2CTURNOUT(2, 2, 150, 195);       
   I2CTURNOUT(3, 3, 150, 195);       
   I2CTURNOUT(4, 4, 150, 195);       
   I2CTURNOUT(5, 5, 150, 195);       
   I2CTURNOUT(6, 6, 150, 195);       
   I2CTURNOUT(7, 9, 150, 195);       
   I2CTURNOUT(8,10, 150, 195);       

   // Sensors

   // Outputs 

   // Signals

 ENDLAYOUT
  
