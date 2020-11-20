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
 #ifndef JMRILayout_H
 #define JMRILayout_H
#include "LayoutManager.h" 

 

class JMRILayout : public LayoutManager {
  public:
       void begin();
       bool setTurnout(byte id, bool left);
       int getSensor(byte id);
       bool setOutput(byte id, bool on);
       bool setSignal(byte id, char RGA);
       bool streamTurnoutList(Print * stream, bool withrottleStyle); // or JMRI style if false
       bool defineTurnout(int id, int addr, byte subaddr);
       bool deleteTurnout(int id);

  private:
};
#endif
