#ifndef TPL2_H
#define TPL2_H

enum OPCODE {OPCODE_TL,OPCODE_TR,
             OPCODE_FWD,OPCODE_REV,OPCODE_SPEED,OPCODE_INVERT_DIRECTION,
             OPCODE_RESERVE,OPCODE_FREE,
             OPCODE_AT,OPCODE_AFTER,OPCODE_SET,OPCODE_RESET,
             OPCODE_IF,OPCODE_IFNOT,OPCODE_ENDIF,OPCODE_IFRANDOM,
             OPCODE_DELAY,OPCODE_RANDWAIT,
             OPCODE_FON, OPCODE_FOFF,
             OPCODE_RED,OPCODE_GREEN,
             OPCODE_PAD,OPCODE_FOLLOW,OPCODE_ENDROUTE,
             OPCODE_PROGTRACK,OPCODE_READ_LOCO1,OPCODE_READ_LOCO2,
             OPCODE_SCHEDULE,OPCODE_SETLOCO,
             OPCODE_PAUSE, OPCODE_RESUME, 
             OPCODE_ROUTE,OPCODE_ENDROUTES
             };


 class TPL2 {
   public:
    static void begin();
    static void loop();
    TPL2(byte route);
    ~TPL2();
    static void readLocoCallback(int cv);
  private: 
    static void ComandFilter(Print * stream, byte & opcode, byte & paramCount, int p[]);
    static bool parseSlash(Print * stream, byte & opcode, byte & paramCount, int p[]) ;

    static int locateRouteStart(short _route);
    static int progtrackLocoId;
    static TPL2 * loopTask;
    static TPL2 * pausingTask;
    void delayMe(int millisecs);
    void driveLoco(byte speedo);
    bool readSensor(short id);
    void skipIfBlock();
    void setSignal(short num, bool go);
    bool readLoco();
    void showManual();
    void showProg(bool progOn);
    bool doManual();
    void loop2();          
   
  static const short SECTION_FLAG = 0x01;
  static const short SENSOR_FLAG = 0x02;
  static const short SIGNAL_FLAG = 0x04;
  static const short MAX_FLAGS=128;
  static byte flags[MAX_FLAGS];
  static byte sensorCount;
  static bool manual_mode;
  static bool manual_mode_flipflop;
  static byte manualTurnoutNumber;
  static int  manualModeCounter;
  static int signalZeroPin;

 // Local variables
    int progCounter;    // Byte offset of next route opcode in ROUTES table
    unsigned long delayStart; // Used by opcodes that must be recalled before completing
    unsigned long waitAfter; // Used by OPCODE_AFTER
    unsigned int  delayTime;
    int loco;
    bool forward;
    bool invert;
    int speedo;
    TPL2 *next;   // loop chain 
};
#endif
