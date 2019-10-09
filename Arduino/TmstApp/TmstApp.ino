/* TmstApp.ino - Two Mode Servo Tester Arduino application 
 * Copyright (C) 2019 by Stefan Grimm
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the prfServo Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <Servo.h>

const uint8_t DINSWITCH = 5;
const uint8_t DOUTSERVO[4]= { 9, 8, 7, 6 };
const uint8_t AINPOT = A0;

enum TmsType {
  Sweep = 0,
  Set
};

class TmsState {
  public:
  virtual TmsType type() = 0;
  virtual void loop() = 0;
};

class TmsSweepState : public TmsState {  
  public:
  TmsType type() { return Sweep; }
  void loop();
  
  private:
  float _angle = 90;
  int8_t _dir = 1;
};

class TmsSetState : public TmsState {  
  public:
  TmsType type() { return Set; }
  void loop();

  private:
  uint8_t _angle = 255;
};

class TmsStateMachine {
  public:
  void loop();

  private:
  TmsState* _state = 0;
  TmsSweepState _sweepState;
  TmsSetState _setState;
};

Servo servo[4];
TmsStateMachine stateMachine;

void setup() {
  for (int n=0; n<4; n++) {
    servo[n].attach(DOUTSERVO[n]);
  }  
  pinMode(DINSWITCH, INPUT);
  Serial.begin(9600);
}

void loop() {
  stateMachine.loop();
  delay(15);
}

void TmsStateMachine::loop() {
  TmsType type = (TmsType)digitalRead(DINSWITCH);
  if (_state == 0 || _state->type() != type) {
    if (type == Sweep) {
      _state = &_sweepState;
    }
    else if (type == Set) {
      _state = &_setState;
    }
    else {
      Serial.println("Error"); delay(100); exit(-1);
    }
  }
  _state->loop();
}

void TmsSetState::loop() {
  // Serial.println("TmsSetState::loop");
  int potPos = analogRead(AINPOT);
  uint8_t angle = map(potPos, 0, 1023, 0, 180);
  if (angle != _angle) {
    for (int n=0; n<4; n++) {
      servo[n].write(angle);
    }
    Serial.println(angle);
    _angle = angle;
  }
}

void TmsSweepState::loop() {
  // Serial.println("TmsSweepState::loop");
  int potPos = analogRead(AINPOT);
  float angle = _angle + (_dir / 512.0 * potPos);
  if (angle <= 0 || angle >= 180) {
    _dir = _dir * -1;
  }
  if ((int16_t)angle != (int16_t)_angle) {
    for (int n=0; n<4; n++) {
      servo[n].write(angle);
    }
    Serial.println(angle);
  }
  _angle = angle;
}
