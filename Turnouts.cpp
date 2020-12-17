/*
 *  © 2013-2016 Gregg E. Berman
 *  © 2020, Chris Harlow. All rights reserved.
 *  © 2020, Harald Barth.
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
#include "Turnouts.h"
#include "EEStore.h"
#include "PWMServoDriver.h"
#ifdef EESTOREDEBUG
#include "DIAG.h"
#endif
//
// Additions for Kato turnout
//
#include <SwitchMachine.h>
#include <RingBuffer.h>
#include "Timeout.h"

RingBuffer swQueue(NumPins*2);  //allows for 2 commands queued per switch
SwitchMachine* swms[NumPins];  // define number switch machines
Timeout timer;
bool tstate = false;

bool Turnout::activate(int n,bool state){
#ifdef EESTOREDEBUG
  DIAG(F("\nTurnout::activate(%d,%d)\n"),n,state);
#endif
  Turnout * tt=get(n);
  if (tt==NULL) return false;
  tt->activate(state);
  EEStore::store();
  turnoutlistHash++;
  return true;
}

bool Turnout::isActive(int n){
  Turnout * tt=get(n);
  if (tt==NULL) return false;
  return tt->data.tStatus & STATUS_ACTIVE;
}

// activate is virtual here so that it can be overridden by a non-DCC turnout mechanism
void Turnout::activate(bool state) {
#ifdef EESTOREDEBUG
  DIAG(F("\nTurnout::activate(%d)\n"),state);
#endif
  if (state)
    data.tStatus|=STATUS_ACTIVE;
  else
    data.tStatus &= ~STATUS_ACTIVE;
  if (data.tStatus & STATUS_PWM)
    PWMServoDriver::setServo(data.tStatus & STATUS_PWMPIN, (data.inactiveAngle+(state?data.moveAngle:0)));
  else if (data.tStatus & STATUS_KATO){
      if (!state)
      swQueue.push((byte) SwitchMachine::eMain | (data.address << 4));
      else
      swQueue.push((byte) SwitchMachine::eDiverging | (data.address << 4));
    }
  else
    DCC::setAccessory(data.address,data.subAddress, state);
  EEStore::store();
}
///////////////////////////////////////////////////////////////////////////////

Turnout* Turnout::get(int n){
  Turnout *tt;
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;tt=tt->nextTurnout);
  return(tt);
}
///////////////////////////////////////////////////////////////////////////////

bool Turnout::remove(int n){
  Turnout *tt,*pp=NULL;

  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextTurnout);

  if(tt==NULL) return false;
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);
  turnoutlistHash++;
  return true; 
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::load(){
  struct TurnoutData data;
  Turnout *tt;

  for(int i=0;i<EEStore::eeStore->data.nTurnouts;i++){
    EEPROM.get(EEStore::pointer(),data);
    if (data.tStatus & STATUS_PWM) tt=create(data.id,data.tStatus & STATUS_PWMPIN, data.inactiveAngle,data.moveAngle);
    else {
      if (data.tStatus & STATUS_KATO) {
        for (byte k = 0; k < NumPins; ++k) {
          swms[k] = new SwitchMachine(pins[k]); // create the switch machine objects
          delay(100);
        }
      data.subAddress = 4; //Kato subaddress > 3 ...make sure
      } 
    bool tstate = false;
    if (data.tStatus & STATUS_ACTIVE) tstate = true;
    tt=create(data.id,data.address,data.subAddress);
    } 
    if (tstate) data.tStatus= data.tStatus | STATUS_ACTIVE; //restore active state
    if(data.tStatus & STATUS_KATO){
      if (data.tStatus & !STATUS_ACTIVE) {     // queue up the commands to initialize the turnout to stored state
       swQueue.push((byte) SwitchMachine::eMain | (data.address << 4)),
       swQueue.push((byte) SwitchMachine::eRefresh | (data.address << 4)); //force it
      }
      else {
       swQueue.push((byte) SwitchMachine::eDiverging | (data.address << 4)),
       swQueue.push((byte) SwitchMachine::eRefresh | (data.address << 4)); //force it
      }
    }
    EEStore::advance(sizeof(tt->data));
#ifdef EESTOREDEBUG
    tt->print(tt);
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::store(){
  Turnout *tt;

  tt=firstTurnout;
  EEStore::eeStore->data.nTurnouts=0;

  while(tt!=NULL){
#ifdef EESTOREDEBUG
    tt->print(tt);
#endif
    EEPROM.put(EEStore::pointer(),tt->data);
    EEStore::advance(sizeof(tt->data));
    tt=tt->nextTurnout;
    EEStore::eeStore->data.nTurnouts++;
  }

}
///////////////////////////////////////////////////////////////////////////////

Turnout *Turnout::create(int id, int add, int subAdd){
  Turnout *tt=create(id);
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  if (subAdd >3) {
    tt->data.subAddress = 4; //force KATO = 4
    tt->data.tStatus = STATUS_KATO;
  }
  return(tt);
}

Turnout *Turnout::create(int id, byte pin, int activeAngle, int inactiveAngle){
  Turnout *tt=create(id);
  tt->data.tStatus= STATUS_PWM | (pin &  STATUS_PWMPIN);
  tt->data.inactiveAngle=inactiveAngle;
  tt->data.moveAngle=activeAngle-inactiveAngle;
  return(tt);
}

Turnout *Turnout::create(int id){
  Turnout *tt=get(id);
  if (tt==NULL) { 
     tt=(Turnout *)calloc(1,sizeof(Turnout));
     tt->nextTurnout=firstTurnout;
     firstTurnout=tt;
     tt->data.id=id;
    }
  turnoutlistHash++;
  return tt;
  }

///////////////////////////////////////////////////////////////////////////////
//
// print debug info about the state of a turnout
//
#ifdef EESTOREDEBUG
void Turnout::print(Turnout *tt) {
    if (tt->data.tStatus & STATUS_PWM )
      DIAG(F("Turnout %d ZeroAngle %d MoveAngle %d Status %d\n"),tt->data.id, tt->data.inactiveAngle, tt->data.moveAngle,tt->data.tStatus & STATUS_ACTIVE);
    else
	DIAG(F("Turnout %d Addr %d Subaddr %d Status %d\n"),tt->data.id, tt->data.address, tt->data.subAddress,tt->data.tStatus & STATUS_ACTIVE);
}
#endif

void Turnout::swUpdate(){ 
  // Called once each pass in loop()
  // Update the SwitchMachine objects.
  for (byte i = 0; i < NumPins; ++i) {
    swms[i]->update();
  }
    
  // Update the Timeout.  ensure only one switch actived at a time
  timer.update();
  
  // If enough time has passed and there are any
  // operation codes in the queue, fetch the code
  // at the front of the queue and command the
  // selected switch machine to throw.
  if (timer.isTimedOut() && !swQueue.isEmpty()) {
    // Fetch the operation code.
    byte op = swQueue.pop();
    
    // The channel number is in the high 4 bits.
    byte chan = (op >> 4) & 0x0F;
    
    // The operation is in the low 4 bits.
    op &= 0x0F;
    
    // Command the switch machine to throw.
    swms[(chan-1)]->throwPoints(static_cast<SwitchMachine::E_DIR>(op));
    
    // Force minimum delay before another switch
    // machine can be commanded to throw.
    timer.setTimeout(100);
  }

} // update switch machine
Turnout *Turnout::firstTurnout=NULL;
int Turnout::turnoutlistHash=0; //bump on every change so clients know when to refresh their lists
