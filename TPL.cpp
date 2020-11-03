#include <Arduino.h>
#include "TPL.h"
#include "TPL2.h"
#include "TPLSensor.h"
#include "Turnouts.h"

void TPL::I2CTURNOUT(byte id,  byte pin, int servoLeft, int servoRight) {
   Turnout::create(id,  pin, servoLeft, servoRight);
}
void TPL::DCCTURNOUT(byte id, int dccAddress, byte subAddress, bool activatedIsLeft) {
  Turnout::create(id, dccAddress, subAddress, activatedIsLeft);
}
void TPL::I2CSENSOR(byte id, byte pin) {
  new TPLSensor(id, true, pin);
}
void TPL::PINSENSOR(byte id, byte pin) {
  new TPLSensor(id, false, pin);
}
