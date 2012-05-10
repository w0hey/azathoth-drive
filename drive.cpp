#include <Arduino.h>
#include <EEPROM.h>
#include "drive.h"
#include "pinout.h"

Drive::Drive() {
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
}

// set X calibration offset
void Drive::setXCenter(byte val) {
  x_center = val;
  center();
}

// set y calibration offset
void Drive::setYCenter(byte val) {
  y_center = val;
  center();
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
  x_position = constrain(x, -70, 70);
  y_position = constrain(y, -70, 70);
  x_value = x_center + x_position;
  y_value = y_center + y_position;
  analogWrite(P_JOY_X, x_value);
  analogWrite(P_JOY_Y, y_value);
}

// return the simulated joystick to center
void Drive::center() {
  setPosition(0, 0);
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
  
