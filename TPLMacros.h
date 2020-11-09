#ifndef TPLMacros_H
#define TPLMacros_H


#define ROUTES const  PROGMEM  byte TPL::RouteCode[] = {
#define ROUTE(id)  OPCODE_ROUTE, id, 
#define ENDROUTE OPCODE_ENDROUTE,0,
#define ENDROUTES OPCODE_ENDROUTES,0 };
 
#define AFTER(sensor_id) OPCODE_AT,sensor_id,OPCODE_AFTER,sensor_id,
#define AT(sensor_id) OPCODE_AT,sensor_id,
#define DELAY(mindelay) OPCODE_DELAY,mindelay,
#define DELAYRANDOM(mindelay,maxdelay) OPCODE_DELAY,mindelay,OPCODE_RANDWAIT,maxdelay-mindelay,
#define ENDIF  OPCODE_ENDIF,0,
#define FOFF(func) OPCODE_FOFF,func,
#define FOLLOW(route) OPCODE_FOLLOW,route,
#define FON(func) OPCODE_FON,func,
#define FREE(blockid) OPCODE_FREE,blockid,
#define FWD(speed) OPCODE_FWD,speed,
#define GREEN(signal_id) OPCODE_GREEN,signal_id,
#define IF(sensor_id) OPCODE_IF,sensor_id,
#define IFNOT(sensor_id) OPCODE_IFNOT,sensor_id,
#define IFRANDOM(percent) OPCODE_IFRANDOM,percent,
#define INVERT_DIRECTION OPCODE_INVERT_DIRECTION,0,
#define PAUSE OPCODE_PAUSE,0,
#define PROGTRACK(onoff) OPCODE_PROGTRACK,onoff,
#define READ_LOCO OPCODE_READ_LOCO1,0,OPCODE_READ_LOCO2,0,
#define RED(signal_id) OPCODE_RED,signal_id,
#define RESERVE(blockid) OPCODE_RESERVE,blockid,
#define RESET(sensor_id) OPCODE_RESET,sensor_id,
#define RESUME OPCODE_RESUME,0,
#define REV(speed) OPCODE_REV,speed,
#define SCHEDULE(route) OPCODE_SCHEDULE,route,
#define SETLOCO(loco) OPCODE_SETLOCO,loco>>7,OPCODE_PAD,loco&0x7F,
#define SET(sensor_id) OPCODE_SET,sensor_id,
#define SPEED(speed) OPCODE_SPEED,speed,
#define STOP OPCODE_SPEED,0, 
#define ESTOP OPCODE_SPEED,1, 
#define TL(id)  OPCODE_TL,id,
#define TR(id)  OPCODE_TR,id,

#endif
