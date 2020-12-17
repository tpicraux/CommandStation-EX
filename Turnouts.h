/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  
 *  This file is part of Asbelos DCC API
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
#ifndef Turnouts_h
#define Turnouts_h

#include <Arduino.h>
#include "DCC.h"
/*
 *Added support for Kato Turnouts, which are not DCC driven, but driven directly from and Arduino
 *Mega 2560 (an UNO may work for only a couple turnouts if the pins can be identified)
 *This implementation requires 3 pins to drive the electronics for one turnout
 *
 * The implementation is based on design and Arduino Libraries developed by Thomas W Rackers
 * (twrackers ...github libraries at ...https://github.com/twrackers)
 * A detailed description of the electronics and code can be found at
 *  ...https://modelrailroadelectronics.blog/switch-machine-controllers/
 * 
 * This implementation using the libraries permits DCC++ EX, etc to drive the Kato switches
 * and does work with JMRI and WiThrottle 
 * The original design of twrackers used I2C to communicate to distributed Arduinos to drive the pins.
 * This implementation does not use I2C, rather it directly drives pins on the Arduino
 * 
*/
#include <SwitchMachine.h>
#include <Triad.h>
// below defines the pins on a mega to drive 10 turnouts...fewer turnouts can be defined and used
// by deleting the unwanted pin groups from the triad below (or defining different pins to be used)
// 
// define all 10 pins sets
const Triad<byte> pins[] = {
    Triad<byte>(24,26,28),
    Triad<byte>(25,27,29),
    Triad<byte>(30,32,34),
    Triad<byte>(31,33,35),
    Triad<byte>(36,38,40),
    Triad<byte>(37,39,41),
    Triad<byte>(42,44,46),
    Triad<byte>(43,45,47),
    Triad<byte>(48,50,52),
    Triad<byte>(49,51,53)
  };  
const byte NumPins = sizeof pins / sizeof *pins;  //max 10 pin groups of 3
//

const byte STATUS_ACTIVE=0x80; // Flag as activated
const byte STATUS_PWM=0x40; // Flag as a PWM turnout = 0x00 if no PWM
// const byte STATUS_PWMPIN=0x3F; // PWM  pin 0-63 
const byte STATUS_PWMPIN=0x1F; // PWM pin 0-31 if using KATO turnout 
// The above limits the PWM servos to 2 boards, with 16 servos
// ... vs 4 boards with 16 pins if no KATO
const byte STATUS_KATO=0x20; //flag for Kato turnouts = 0x00 if no Kato
// carefull choice of id allows all 3 types of turnouts...

struct TurnoutData {
   int id;
   uint8_t tStatus; // has STATUS_ACTIVE, STATUS_PWM, STATUS_KATO, STATUS_PWMPIN  
   union {uint8_t subAddress; char moveAngle;}; //DCC  sub addrerss or PWM difference from inactiveAngle  
   union {int address; int inactiveAngle;}; // DCC address or PWM servo angle 
};

class Turnout {
  public:
  static Turnout *firstTurnout;
  static int turnoutlistHash;
  TurnoutData data;
  Turnout *nextTurnout;
  static  bool activate(int n, bool state);
  static Turnout* get(int);
  static bool remove(int);
  static bool isActive(int);
  static void load();
  static void store();
  static void swUpdate();
  static Turnout *create(int id , int address , int subAddress); //Kato or DCC accessory turnout (for Kato- subaddress > 3)
  static Turnout *create(int id , byte pin , int activeAngle, int inactiveAngle); //PWM I2C turnout
  static Turnout *create(int id);
  void activate(bool state);
#ifdef EESTOREDEBUG
  void print(Turnout *tt);
#endif
}; // Turnout
  
#endif
