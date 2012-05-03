#include <EEPROM.h>

#include "pinout.h"
#include "link.h"

// Default calibration values
const byte x_def = 127;
const byte y_def = 127;

Link link = Link(dispatch_packet);
byte eeprom_x = 0;
byte eeprom_y = 0;
byte x_center = 0;
byte y_center = 0;


void setup() {
  // Adjust time 1 for higher frequency PWM
  TCCR1B = TCCR1B & 0b11111000 | 0x01; // 31250 Hz
  pinMode(P_JOY_X, OUTPUT);
  pinMode(P_JOY_Y, OUTPUT);
  loadCalibration();
  // Center the simulated joystic (hopefully)
  analogWrite(P_JOY_X, x_center); // hopefully about 2.5v, the centered position.
  analogWrite(P_JOY_Y, y_center);
  Serial.begin(115200);

}

void loop() {
  return;
  
}

void serialEvent() {
  link.service();
}

void dispatch_packet(int length, byte* packet) {
  switch (packet[2]) {
    case 0x30:
      cmd_joystick(length, packet);
      break;
    case 0x40:
      cmd_calibrate(length, packet);
      break;
    case 0x41:
      cmd_get_calibration();
      break;
    case 0xF0:
      // soft stop
      joystickCenter();
      break;
    default:
      break;
  }
}

void cmd_joystick(int length, byte* packet) {
  // Expects two signed 8-bit values, indicating the desired joystick
  // position relative to center.
  char xcmd = packet[3];
  char ycmd = packet[4];
  byte xpos = x_center + xcmd;
  byte ypos = y_center + ycmd;
  analogWrite(P_JOY_X, xpos);
  analogWrite(P_JOY_Y, ypos);
  return;
}

void cmd_calibrate(int length, byte* packet) {
  switch (packet[3]) {
    case 0x00:
      // Set x center
      x_center = packet[4];
      joystickCenter();
      break;
    case 0x01:
      // set y center
      y_center = packet[4];
      joystickCenter();
      break;
    case 0x10:
      // Write calibration to EEPROM
      writeCalibration();
      break;
  }
}

void cmd_get_calibration() {
  byte data[5];
  data[0] = 0x41;
  data[1] = x_center;
  data[2] = y_center;
  data[3] = eeprom_x;
  data[4] = eeprom_y;
  link.sendData(5, data);
}

void loadCalibration() {
  byte eeprom_magic = EEPROM.read(0);
  if (eeprom_magic = 0x23) {
    // EEPROM has values stored
    eeprom_x = EEPROM.read(1);
    eeprom_y = EEPROM.read(2);
    x_center = eeprom_x;
    y_center = eeprom_y;
  }
  else {
    // No magic number, EEPROM is probably empty/invalid
    // Use defaults instead
    x_center = x_def;
    y_center = y_def;
  }
}

void writeCalibration() {
  EEPROM.write(1, x_center);
  eeprom_x = x_center;
  EEPROM.write(2, y_center);
  eeprom_y = y_center;
  EEPROM.write(0, 0x23);
}

void joystickCenter() {
  analogWrite(P_JOY_X, x_center);
  analogWrite(P_JOY_Y, y_center);
}
