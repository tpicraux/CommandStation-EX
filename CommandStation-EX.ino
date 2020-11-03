////////////////////////////////////////////////////////////////////////////////////
//  © 2020, Chris Harlow. All rights reserved.
//
//  This file is a demonstattion of setting up a DCC-EX
// Command station with optional support for direct connection of WiThrottle devices
// such as "Engine Driver". If you contriol your layout through JMRI
// then DON'T connect throttles to this wifi, connect them to JMRI.
//
//  THE WIFI FEATURE IS NOT SUPPORTED ON ARDUINO DEVICES WITH ONLY 2KB RAM.
////////////////////////////////////////////////////////////////////////////////////

#include "config.h"
#include "DCCEX.h"

// All RF24 feature includes
#include <SPI.h> // RF24
#include <RF24.h>
#include <RF24Network.h>

// more RF24 Feature setup
RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
//RF24 radio(10, 9);             // Alt config
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 05;
const uint16_t master_node00 = 00;

// ----------------- RF24 Feature -------------
// This is an example of the DCC-EX 3.0 User Override Function (aka filters)
// The filter must be enabled by calling the DCC EXParser::setFilter method, see use in setup().
#include "Turnouts.h" 
void RF24filter(Print * stream, byte & opcode, byte & paramCount, int p[]) {
    bool result;
    (void)stream; // avoid compiler warning if we don't access this parameter
    if (opcode=='T' && paramCount==2) {  // look for command of format: <T id 1/0>  
       //  This checks if a turnout was created. To bypass check, remove these 2 lines
       //  and the include for Turnouts.h above
       Turnout * tt= Turnout::get(p[0]); // Locate turnout definition 
       if (!tt) return; // No turnout found
       
       // You have access to tt->data.address and tt->subAddress 

       // I'm guessing that the Node is constant here 
       RF24NetworkHeader header(master_node00);
       char data[20]="/1/";
       itoa(p[0],data+3,10); // convert int to string. Is there an override of rf24 functions to avoid this?
       strcat(data,p[1]==1?"/1":"/2"); // build the command. p[0] is id, p[1] is throw/close
       result = network.write(header, data, strlen(data));     
       // StringFormatter::send(stream, F("<H %d %d>"), tt->data.id, p[1]);
       if (result){
         StringFormatter::send(stream, F("<H %d %d>"), p[0], p[1]);
       }
       else {
         StringFormatter::send(stream, F("<X>"));
       }
       
       opcode=0;  // tell parser to ignore this command as we have done it already
    }
}

// Create a serial command parser for the USB connection, 
// This supports JMRI or manual diagnostics and commands
// to be issued from the USB serial console.
DCCEXParser serialParser;

void setup()
{
  // The main sketch has responsibilities during setup()

  // Responsibility 1: Start the usb connection for diagnostics
  // This is normally Serial but uses SerialUSB on a SAMD processor
  Serial.begin(115200);
  DIAG(F("DCC++ EX v%S"),F(VERSION));
   
  CONDITIONAL_LCD_START {
    // This block is ignored if LCD not in use 
    LCD(0,F("DCC++ EX v%S"),F(VERSION));
    LCD(1,F("Starting")); 
    }   

//  Start the WiFi interface on a MEGA, Uno cannot currently handle WiFi

#if WIFI_ON
  WifiInterface::setup(WIFI_SERIAL_LINK_SPEED, F(WIFI_SSID), F(WIFI_PASSWORD), F(WIFI_HOSTNAME), IP_PORT);
#endif // WIFI_ON

  // Responsibility 3: Start the DCC engine.
  // Note: this provides DCC with two motor drivers, main and prog, which handle the motor shield(s)
  // Standard supported devices have pre-configured macros but custome hardware installations require
  //  detailed pin mappings and may also require modified subclasses of the MotorDriver to implement specialist logic.

  // STANDARD_MOTOR_SHIELD, POLOLU_MOTOR_SHIELD, FIREBOX_MK1, FIREBOX_MK1S are pre defined in MotorShields.h

  // Optionally a Timer number (1..4) may be passed to DCC::begin to override the default Timer1 used for the
  // waveform generation.  e.g.  DCC::begin(STANDARD_MOTOR_SHIELD,2); to use timer 2

  DCC::begin(MOTOR_SHIELD_TYPE); 
  LCD(1,F("Ready")); 
  
  // rf24 feature - Enable the filter
  // if we have a display, optionally do something like this: LCD(2,F("RF24 Active"))
  DCCEXParser::setFilter(RF24filter); 
  SPI.begin();
  radio.begin();
  network.begin(90, this_node); //(channel, node address)
  radio.setDataRate(RF24_2MBPS); // Max baud rate
}

void loop()
{
  // The main sketch has responsibilities during loop()

  // Responsibility 1: Handle DCC background processes
  //                   (loco reminders and power checks)
  DCC::loop();

  // Responsibility 2: handle any incoming commands on USB connection
  serialParser.loop(Serial);

// Responsibility 3: Optionally handle any incoming WiFi traffic
#if WIFI_ON
  WifiInterface::loop();
#endif

  LCDDisplay::loop();  // ignored if LCD not in use 
  
// Optionally report any decrease in memory (will automatically trigger on first call)
#if ENABLE_FREE_MEM_WARNING
  static int ramLowWatermark = 32767; // replaced on first loop 

  int freeNow = freeMemory();
  if (freeNow < ramLowWatermark)
  {
    ramLowWatermark = freeNow;
    LCD(2,F("Free RAM=%5db"), ramLowWatermark);
  }
#endif
}
