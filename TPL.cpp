#include <Arduino.h>
#include "TPL.h"
#include "TPLLayout.h"
#include "DCC.h"
#include "DIAG.h"
#include "WiThrottle.h"
#include "DCCEXParser.h"


// Command parsing keywords
const int HASH_KEYWORD_TPL=23368;    
const int HASH_KEYWORD_ON = 2657;
const int HASH_KEYWORD_SCHEDULE=-9179;
const int HASH_KEYWORD_RESERVE=11392;
const int HASH_KEYWORD_FREE=-23052;
const int HASH_KEYWORD_TL=2712;
const int HASH_KEYWORD_TR=2694;
const int HASH_KEYWORD_SET=27106;
const int HASH_KEYWORD_RESET=26133;
const int HASH_KEYWORD_PAUSE=-4142;
const int HASH_KEYWORD_RESUME=27609;
const int HASH_KEYWORD_STATUS=-25932;


// Statics 
byte TPL::flags[MAX_FLAGS];

int TPL::progtrackLocoId;


TPL * TPL::loopTask=NULL; // loopTask contains the address of ONE of the tasks in a ring.
TPL * TPL::pausingTask=NULL; // Task causing a PAUSE. 
 // when pausingTask is set, that is the ONLY task that gets any service,
 // and all others will have their locos stopped, then resumed after the pausing task resumes.

TPL::TPL(byte route) {
  progCounter=locateRouteStart(route);
  delayTime=0;
  loco=0;
  speedo=0;
  forward=true;

  // chain into ring of TPLs
  if (loopTask==NULL) {
    loopTask=this;
    next=this;
  }
  else {
        next=loopTask->next;
        loopTask->next=this;
  }
  
  if (Diag::TPL) DIAG(F("\nTPL created for Route %d at prog %d, next=%x, loopTask=%x\n"),route,progCounter,next,loopTask);
}
TPL::~TPL() {
  if (next==this) loopTask=NULL;
  else for (TPL* ring=next;;ring=ring->next) if (ring->next == this) {
           ring->next=next;
           loopTask=next;
           break;
       }
}


int TPL::locateRouteStart(short _route) {
  if (_route==0) return 0; // Route 0 is always start of ROUTES for default startup 
  for (int pcounter=0;;pcounter+=2) {
    byte opcode=pgm_read_byte_near(TPL::RouteCode+pcounter);
    if (opcode==OPCODE_ENDROUTES) return -1;
    if (opcode==OPCODE_ROUTE) if( _route==pgm_read_byte_near(TPL::RouteCode+pcounter+1)) return pcounter;
  }
}

/* static */ void TPL::begin() {
  DIAG(F("\nTPL begin\n"));
  DCCEXParser::setFilter(TPL::ComandFilter);
  new TPL(0); // add the startup route
  
  TPLLayout::begin();// set pin modes for sensors, outputs, signals 
  DIAG(F("\nTPL ready\n"));
}


// This filter intercepst <> commands to do the following:
// - Implement TPL specific commands/diagnostics 
// - Reject/modify JMRI commands that would interfere with TPL processing 
void TPL::ComandFilter(Print * stream, byte & opcode, byte & paramCount, int p[]) {
    (void)stream; // avoid compiler warning if we don't access this parameter 
    bool reject=false;
    switch(opcode) {
        
     case 'S': // Reject all JMRI sensor commands
     case 'Q': // Reject all JMRI sensor commands
     case 'Z': // Reject all output commands
     case 'E': // Reject all EEPROM commands
     case 'e': // Reject all EEPROM commands
          reject=true;
          break;

     case 'D':
        if (p[0]==HASH_KEYWORD_TPL) { // <D TPL ON/OFF>
           Diag::TPL = paramCount==2 && (p[1]==HASH_KEYWORD_ON || p[1]==1);
           opcode=0;
        }
        break;

     case 'T':      
           if (paramCount!=2) { // Reject all Turnout define/delete commands
               reject=true;
               break;
           }
           TPLLayout::setTurnout(p[0],p[1]);
           opcode=0;
           break;

      case 't': // THROTTLE <t [REGISTER] CAB SPEED DIRECTION>          
          // TODO - Monitor throttle commands and reject any that are in current automation
          break;
          
     case '/':  // New TPL command
           reject=!parseSlash(stream,paramCount,p);
          opcode=0;
          break;
          
     default:  // other commands pass through 
     break;       
      }
     if (reject) {
      opcode=0;
      if (Diag::TPL) DIAG(F("\nTPL rejects <%c>"),opcode); 
      StringFormatter::send(stream,F("<X>"));
     }
}
     
bool TPL::parseSlash(Print * stream, byte & paramCount, int p[]) {
          
          switch (p[0]) {
            case HASH_KEYWORD_PAUSE: // </ PAUSE>
                 if (paramCount!=1) return false;
                 DCC::setThrottle(0,1,true);  // pause all locos on the track         
                 pausingTask=(TPL *)1; // Impossible task address
                 return true;
                 
            case HASH_KEYWORD_RESUME: // </ RESUME>
                 if (paramCount!=1) return false;
                 pausingTask=NULL;
                 return true;
                 
            case HASH_KEYWORD_STATUS: // </STATUS>
                 if (paramCount!=1) return false;
                 StringFormatter::send(stream, F("\nTPL STATUS"));
                 {
                  TPL * task=loopTask;
                  while(task) {
                      StringFormatter::send(stream,F("\nPC=%d,DT=%d,LOCO=%d%c,SPEED=%d%c"),
                            task->progCounter,task->delayTime,task->loco,
                            task->invert?'I':' ',
                            task->speedo, 
                            task->forward?'F':'R'
                            );
                      task=task->next;      
                      if (task==loopTask) break;      
                    }                            
                 }
                 for (int i=0;i<MAX_FLAGS; i++) {
                   byte flag=flags[i];
                   if (flag) {
                     StringFormatter::send(stream,F("\nflags[%d} "),i);
                     if (flag & SECTION_FLAG) StringFormatter::send(stream,F(" RESERVED"));
                     if (flag & TURNOUT_FLAG_LEFT) StringFormatter::send(stream,F(" TL"));
                     if (flag & TURNOUT_FLAG_RIGHT) StringFormatter::send(stream,F(" TR"));
                     if (flag & SENSOR_FLAG) StringFormatter::send(stream,F(" SET"));
                     if ((flag & SIGNAL_FLAG_AMBER) ==SIGNAL_FLAG_AMBER) 
                          StringFormatter::send(stream,F(" AMBER"));
                     else if (flag & SIGNAL_FLAG_RED) 
                          StringFormatter::send(stream,F(" RED"));
                     else if (flag & SIGNAL_FLAG_GREEN) 
                          StringFormatter::send(stream,F(" GREEN"));
                   }                 
                 }
                 StringFormatter::send(stream,F("\n"));        
                 return true;
                 
            case HASH_KEYWORD_SCHEDULE: // </ SCHEDULE [cab] route >
                 if (paramCount<2 || paramCount>3) return false;
                 {                 
                  TPL * newt=new TPL((paramCount==2) ? p[1] : p[2]);
                  newt->loco=(paramCount==2)? 0 : p[1];                    
                  newt->speedo=0;
                  newt->forward=true;
                  newt->invert=false;
                 }
              return true;
                 
            default:
              break;
          }
          
          // all other / commands take 1 parameter 0 to MAX_FLAGS-1     

          if (paramCount!=2 || p[1]<0  || p[1]>=MAX_FLAGS) return false;

          switch (p[0]) {     
            case HASH_KEYWORD_RESERVE:  // force reserve a section
                 setFlag(p[1],SECTION_FLAG,0);
                 return true;
    
            case HASH_KEYWORD_FREE:  // force free a section
                 setFlag(p[1],0,SECTION_FLAG);
                 return true;
                 
            case HASH_KEYWORD_TL:  // force Turnout LEFT
                 TPLLayout::setTurnout(p[1], true);
                 setFlag(p[1], TURNOUT_FLAG_LEFT, TURNOUT_FLAG_RIGHT);
                return true;
                 
            case HASH_KEYWORD_TR:  // Force Turnout RIGHT
                 TPLLayout::setTurnout(p[1], false);
                 setFlag(p[1], TURNOUT_FLAG_RIGHT, TURNOUT_FLAG_LEFT);
                 return true;
                
            case HASH_KEYWORD_SET:
                 setFlag(p[1], SENSOR_FLAG,0);
                 return true;
   
            case HASH_KEYWORD_RESET:
                 setFlag(p[1], 0, SENSOR_FLAG);
                 return true;
                  
            default:
                 return false;                 
          }
    }

void TPL::setFlag(byte id,byte onMask, byte offMask) {
  
   if (FLAGOVERFLOW(id)) return; // Outside UNO range limit
   byte f=flags[id];
   f &= ~offMask;
   f |= onMask;
   if (f!=flags[id]) {
       flags[id]=f;
       // TODO EEPROM STORE FLAG STATE
   }
}

void TPL::driveLoco(byte speed) {
     if (loco<0) return;  // Caution, allows broadcast! 
     DCC::setThrottle(loco,speed, forward^invert);
     // TODO... if broadcast speed 0 then pause all other tasks. 
}

bool TPL::readSensor(short id) {
  if (FLAGOVERFLOW(id)) return false;
  if (flags[id] & SENSOR_FLAG) return true; // sensor locked on by software
  short s= TPLLayout::getSensor(id); // real hardware sensor (-1 if not exists )
  if (s==1 && Diag::TPL) DIAG(F("\nTPL Sensor %d hit\n"),id);
  return s==1;
}

void TPL::skipIfBlock() {
  short nest = 1;
  while (nest > 0) {
    progCounter += 2;
    byte opcode =  pgm_read_byte_near(TPL::RouteCode+progCounter);;
    switch(opcode) {
      case OPCODE_IF:
      case OPCODE_IFNOT:
      case OPCODE_IFRANDOM:
           nest++;
           break;
      case OPCODE_ENDIF:
           nest--;
           break;
      default:
      break;
    }
  }
}



/* static */ void TPL::readLocoCallback(int cv) {
     progtrackLocoId=cv;
}

void TPL::loop() {
     //DIAG(F("\n+ pausing=%x, looptask=%x"),pausingTask,loopTask);
  
  // Round Robin call to a TPL task each time 
     if (loopTask==NULL) return; 
     
     loopTask=loopTask->next;
     // DIAG(F(" next=%x"),loopTask);
  
     if (pausingTask==NULL || pausingTask==loopTask) loopTask->loop2();
}    

  
void TPL::loop2() {
   if (delayTime!=0 && millis()-delayStart < delayTime) return;
     
  byte opcode = pgm_read_byte_near(TPL::RouteCode+progCounter);
  byte operand =  pgm_read_byte_near(TPL::RouteCode+progCounter+1);
   
  // Attention: Returning from this switch leaves the program counter unchanged.
  //            This is used for unfinished waits for timers or sensors.
  //            Breaking from this switch will step to the next step in the route. 
  switch (opcode) {
    
    case OPCODE_TL:
         TPLLayout::setTurnout(operand, true);
         setFlag( operand, TURNOUT_FLAG_LEFT, TURNOUT_FLAG_RIGHT);
         break;
          
    case OPCODE_TR:
         TPLLayout::setTurnout(operand, false);
         setFlag( operand, TURNOUT_FLAG_RIGHT, TURNOUT_FLAG_LEFT);
         break; 
    
    case OPCODE_REV:
      forward = false;
      driveLoco(speedo);
      break;
    
    case OPCODE_FWD:
      forward = true;
      driveLoco(speedo);
      break;
      
    case OPCODE_SPEED:
      driveLoco(operand);
      break;
    
    case OPCODE_INVERT_DIRECTION:
      invert= !invert;
      driveLoco(speedo);
      break;
      
    case OPCODE_RESERVE:
        if (FLAGOVERFLOW(operand)) break; // CAUTION reserve will not be done
        if (flags[operand] & SECTION_FLAG) {
        driveLoco(0);
        delayMe(500);
        return;
      }
      setFlag(operand,SECTION_FLAG,0);
      break;
    
    case OPCODE_FREE:
      setFlag(operand,0,SECTION_FLAG);
      break;
    
    case OPCODE_AT:
      if (readSensor(operand)) break;
      delayMe(50);
      return;
    
    case OPCODE_AFTER: // waits for sensor to hit and then remain off for 0.5 seconds. (must come after an AT operation)
      if (readSensor(operand)) {
        // reset timer to half a second and keep waiting
        waitAfter=millis();
        return; 
      }
      if (millis()-waitAfter < 500 ) return;   
      break;
    
    case OPCODE_SET:
      setFlag(operand,SENSOR_FLAG,0);
      break;
    
    case OPCODE_RESET:
      setFlag(operand,0,SENSOR_FLAG);
      break;

    case OPCODE_PAUSE:
         DCC::setThrottle(0,1,true);  // pause all locos on the track
         pausingTask=this;
         break;
 
    case OPCODE_RESUME:
         pausingTask=NULL;
         driveLoco(speedo);
         for (TPL * t=next; t!=this;t=t->next) if (t->loco >0) t->driveLoco(t->speedo);
          break;        
    
    case OPCODE_IF: // do next operand if sensor set
      if (!readSensor(operand)) skipIfBlock();
      break;
    
    case OPCODE_IFNOT: // do next operand if sensor not set
      if (readSensor(operand)) skipIfBlock();
      break;
   
    case OPCODE_IFRANDOM: // do block on random percentage
      if (random(100)>=operand) skipIfBlock();
      break;
    
    case OPCODE_ENDIF:
      break;
    
    case OPCODE_DELAY:
      delayMe(operand*10);
      break;
    
    case OPCODE_RANDWAIT:
      delayMe((int)random(operand*10));
      break;
    
    case OPCODE_RED:
      TPLLayout::setSignal(operand,'R');
      setFlag(operand,SIGNAL_FLAG_RED,SIGNAL_FLAG_GREEN);
      break;
    
    case OPCODE_AMBER:
      TPLLayout::setSignal(operand,'A');
      setFlag(operand,SIGNAL_FLAG_AMBER,0);
      break;
    
    case OPCODE_GREEN:
      TPLLayout::setSignal(operand,'G');
      setFlag(operand,SIGNAL_FLAG_GREEN,SIGNAL_FLAG_RED);      
      break;
       
    case OPCODE_FON:      
      DCC::setFn(loco,operand,true);
      break;
    
    case OPCODE_FOFF:
      DCC::setFn(loco,operand,false);
      break;

    case OPCODE_FOLLOW:
      progCounter=locateRouteStart(operand);
      if (progCounter<0) delete this; 
      return;
      
    case OPCODE_ENDROUTE:
    case OPCODE_ENDROUTES:
      delete this;  // removes this task from the ring buffer
      return;
      
    case OPCODE_PROGTRACK:
       if (operand>0) {
        // TODO TPLDCC1::setProgtrackToMain(false);
        // showProg(true);
       }
       else {
            // TODO TPLDCC1::setProgtrackToMain(true);
            // showProg(false);
       }
       break;
       
      case OPCODE_READ_LOCO1: // READ_LOCO is implemented as 2 separate opcodes
       progtrackLocoId=-1;
       DCC::getLocoId(readLocoCallback);
       break;
      
      case OPCODE_READ_LOCO2:
       if (progtrackLocoId<0) {
        delayMe(100);
        return; // still waiting for callback
       }
       loco=progtrackLocoId;
       speedo=0;
       forward=true;
       invert=false;
       break;
       
       case OPCODE_SCHEDULE:
           {
            // Create new task and transfer loco.....
            // but cheat by swapping prog counters with new task  
            new TPL(operand);
            int swap=loopTask->progCounter;
            loopTask->progCounter=progCounter+2;
            progCounter=swap;
           }
           break;
       
       case OPCODE_SETLOCO:
           {
            // two bytes of loco address are in the next two OPCODE_PAD operands
             int operand2 =  pgm_read_byte_near(TPL::RouteCode+progCounter+3);
             progCounter+=2; // Skip the extra two instructions
             loco=operand<<7 | operand2;
             speedo=0;
             forward=true;
             invert=false;
             if (Diag::TPL) DIAG(F("\nTPL SETLOCO %d \n"),loco);
            }
       break;
       
       case OPCODE_ROUTE:
          if (Diag::TPL) DIAG(F("\nTPL Starting Route %d\n"),operand);
          break;

       case OPCODE_PAD:
          // Just a padding for previous opcode needing >1 operad byte.
       break;
    
    default:
      DIAG(F("\nTPL Opcode %d not supported\n"),opcode);
    }
    // Falling out of the switch means move on to the next opcode
    progCounter+=2;
}

void TPL::delayMe(int delay) {
     delayTime=delay;
     delayStart=millis();
}
