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
#include "networkFunctions.h" // optional is using polling

// RF24 System Identifiers
#define SYS_ID "Master Control Node"
#define NODE_CMD_VERSION "1.001.001"
#define NUM_CHILDREN 1

// RF24 Node Addresses
//const uint16_t this_node = 00; 
// Children
const uint16_t node_05 = 05;
uint16_t children[NUM_CHILDREN] = {node_05};

// more RF24 Feature setup
RF24 radio(9, 10);               // nRF24L01 (CE,CSN)
//RF24 radio(10, 9);             // nRF24L01 (CE,CSN)for Nano on Exp Shield
RF24Network network(radio);      // Include the radio in the network

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
       DIAG(F("\nGot T\n"));
       Turnout * tt= Turnout::get(p[0]); // Locate turnout definition 
       if (!tt) return; // No turnout found
       
       // You have access to tt->data.address and tt->subAddress 

       // I'm guessing that the Node is constant here 
       RF24NetworkHeader header(this_node);
       char data[20]="/1/";
       itoa(p[0],data+3,10); // convert int to string. Is there an override of rf24 functions to avoid this?
       strcat(data,p[1]==1?"/1":"/2"); // build the command. p[0] is id, p[1] is throw/close
       result = network.write(header, data, strlen(data));     
       // StringFormatter::send(stream, F("<H %d %d>"), tt->data.id, p[1]);
       DIAG(F("data: %s"),data);
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
  randomSeed(analogRead(A7));
  Serial.println(String(F(SYS_ID)) + String(F(" - SW:")) + String(F(VERSION)));
  Serial.println(F("RF24 start")); 
  SPI.begin();
  radio.begin();
  network.begin(90, this_node); //(channel, node address)
  radio.setDataRate(RF24_2MBPS); // Max baud rate
  String readytxt = String(F("Ready @ node ")) + String(this_node, OCT);
  Serial.println(readytxt);
}

void loop()
{

// poll the network. if no actual packet, pkt.function == "0"
// Function- 1=Turnout Control  2 = Signalling  3 = Indicators 255 = testing
// option- ID param
// data- ID result
/*
  String function, option, data;
  PKT_DEF pkt = pollNet();
  String delimiter = F("/"); // Packet delimiter character
  // process the packet
  // pkt.function == 0 will fall through
  int func = pkt.function.toInt();
  if (func) {
    switch (func) {
      case 1:
        Serial.println(F("Turnout control requested."));
        break;
      case 255: // Test communications
        doCommTest();
        break;
      default:
        Serial.println(String(F("Function ")) + pkt.function + " is currently unavailable.");
        break;
    }
  }
  */
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

bool doCommTest(){
  String delimiter = F("/");
  Serial.println(F("Entering communications test mode."));
  long rdata = 0;
  int func, respData;
  unsigned long cycleStart, waitStart, waitTimer, totalElapsed, waitElapsed;
  bool pingReceived, timeout, dataMatch;
  for(int i = 0; i < NUM_CHILDREN; i++){
    rdata = random(1001, 9999);
    Serial.println(String(F(">> Pinging node ")) + String(children[i], OCT));
    cycleStart = millis();
    sendPacket(children[i], "255", String(rdata), "");
    waitStart = millis();
    timeout = false;
    pingReceived = false;
    while(!pingReceived && !timeout){
      PKT_DEF pkt = pollNet();
      func = pkt.function.toInt();
      waitTimer = millis();
      waitElapsed = waitTimer - waitStart;
      if (func) {
        if(pkt.source_node == children[i]){
          switch (func) {
            case 255: // ping response
              
              pingReceived = true;
              totalElapsed = waitTimer - cycleStart;
              
              respData = pkt.data.toInt();
              dataMatch = (respData == (int) rdata);
              Serial.println(String(F("Ping response received from node ")) + String(pkt.source_node, OCT));
              if(dataMatch){
                Serial.println(String(F("The node returned the correct confirmation code.")));
              } else {
                Serial.println(String(F("The node DID NOT return the correct confirmation code.")));
              }
              Serial.println(String(F("It took ")) + String(waitElapsed) + F(" milliseconds to receive a response from the node."));
              Serial.println(String(F("The communications cycle took ")) + String(totalElapsed) + F(" milliseconds.\n"));
              break;
            default:
              Serial.println(String(F("Unexpected function received from node ")) + String(pkt.source_node, OCT) + String(F(": ")) + pkt.function + delimiter + pkt.option + delimiter + pkt.data);
              break;
          }
        } else {
          Serial.println(String(F("Unexpected packet received from node ")) + String(pkt.source_node, OCT) + F(": ") + pkt.function + delimiter + pkt.option + delimiter + pkt.data);
        }
      }
      if(waitElapsed > 5000){
        timeout = true;
        Serial.println(String(F("Timed out waiting for response from node ")) + String(children[i], OCT));
      }
    }
  }
  Serial.println(F("Communications test complete.\n\n"));

}
