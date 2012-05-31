#include <Arduino.h>
#include <EEPROM.h>
#include "drive.h"
#include "pinout.h"

Drive::Drive(void (*func)()) {
  callback = func;
  pinMode(P_SELECT_OUT, OUTPUT);
  pinMode(P_ESTOP_OUT, OUTPUT);
  pinMode(P_SELECT_IN, INPUT);
  pinMode(P_ESTOP_IN, INPUT);
  // init to default state, manual control, e-stop inactive.
  digitalWrite(P_SELECT_OUT, LOW);
  digitalWrite(P_ESTOP_OUT, HIGH);
  status = 0;
  if (!readCalibration()) {
    x_center = X_DEFAULT;
    y_center = Y_DEFAULT;
  }
  x_position = 0;
  y_position = 0;
  x_value = x_center;
  y_value = y_center;
  analogWrite(P_JOY_X, x_value);
  analogWrite(P_JOY_Y, y_value);
  //updateStatus();
}

// set calibration offsets
void Drive::setCenter(byte xval, byte yval) {
  x_center = xval;
  y_center = yval;
  setPosition(0, 0);
}

// write calibration values to eeprom
void Drive::storeCalibration() {
  eeprom_x = x_center;
  eeprom_y = y_center;
  EEPROM.write(ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.write(ADDR_X, eeprom_x);
  EEPROM.write(ADDR_Y, eeprom_y);
}

// reset eeprom contents to 'empty' (255)
void Drive::eraseCalibration() {
  eeprom_x = 0;
  eeprom_y = 0;
  EEPROM.write(ADDR_MAGIC, 0xff);
  EEPROM.write(ADDR_X, 0xff);
  EEPROM.write(ADDR_Y, 0xff);
}

// set simulated joystick position
// char x, char y: desired position relative to center
void Drive::setPosition(char x, char y) {
  x_position = constrain(map(x, -128, 127, -70, 70), -70, 70);
  y_position = constrain(map(y, -128, 127, -70, 70), -70, 70);
  x_value = x_center + x_position;
  y_value = y_center + y_position;
  analogWrite(P_JOY_X, x_value);
  analogWrite(P_JOY_Y, y_value);
  // quick test to see if either value is nonzero
  if (x || y) {
    status |= STATUS_MOVING;
  } else {
    status &= ~STATUS_MOVING;
  }
  callback();
}

void Drive::select(boolean enabled) {
  if (enabled) {
    digitalWrite(P_SELECT_OUT, HIGH);
    status |= STATUS_SELECT_OUT;
  }
  else {
    digitalWrite(P_SELECT_OUT, LOW);
    status &= ~STATUS_SELECT_OUT;
  }
  callback();
}

void Drive::estop() {
  digitalWrite(P_ESTOP_OUT, LOW);
  status |= STATUS_ESTOP_OUT;
  setPosition(0, 0);
}

void Drive::reset() {
  setPosition(0, 0);
  digitalWrite(P_ESTOP_OUT, HIGH);
  status &= ~STATUS_ESTOP_OUT;
  callback();
}

// get the current simulated joystick position
// returns a pointer to a two-element array
char* Drive::getPosition() {
  static char pos[2];
  pos[0] = x_position;
  pos[1] = y_position;
  return pos;
}

// get the actual PWM output values
// returns a pointer to a two-element array
byte* Drive::getRawPosition() {
  static byte val[2];
  val[0] = x_value;
  val[1] = y_value;
  return val;
}

byte Drive::getStatus() {
  return status;
}

// do some status checks
void Drive::update() {
  byte prevstate = status;
  byte estop_in = digitalRead(P_ESTOP_IN);
  byte select_in = digitalRead(P_SELECT_IN);
  if (estop_in == HIGH) {
    status |= STATUS_ESTOP_IN;
  }
  else if (estop_in == LOW) {
    status &= ~STATUS_ESTOP_IN;
  }
  
  if (select_in == HIGH) {
    status |= STATUS_SELECT_IN;
  }
  else if (select_in == LOW) {
    status &= ~STATUS_SELECT_IN;
  }
  
  if (status != prevstate) {
    // notify that state changed
    callback();
  }
}


// get the current calibration values
// returns a pointer to a four-element array
// { current x, current y, eeprom x, eeprom y }
byte* Drive::getCalibration() {
  static byte val[4];
  val[0] = x_center;
  val[1] = y_center;
  val[2] = eeprom_x;
  val[3] = eeprom_y;
  return val;
}

boolean Drive::readCalibration() {
  if (EEPROM.read(ADDR_MAGIC) == EEPROM_MAGIC) {
    eeprom_x = EEPROM.read(ADDR_X);
    eeprom_y = EEPROM.read(ADDR_Y);
    x_center = eeprom_x;
    y_center = eeprom_y;
    return true;
  }
  return false;
}
  
