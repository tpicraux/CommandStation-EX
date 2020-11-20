/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  
 *  This file is part of CommandStation-EX
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
 #ifndef LayoutManager_H
 #define LayoutManager_H

class LayoutManager {
  public:
      static LayoutManager * manager;
      virtual void begin()=0;
      virtual bool setTurnout(byte id, bool left)=0;
      virtual int  getSensor(byte id)=0;
      virtual bool setOutput(byte id, bool on)=0;
      virtual bool setSignal(byte id, char RGA)=0;
      virtual bool streamTurnoutList(Print * stream, bool withrottleStyle)=0; // or JMRI style if false
      virtual bool defineTurnout(int id, int addr, byte subaddr)=0;
      virtual bool deleteTurnout(int id)=0;
       
};
#endif
