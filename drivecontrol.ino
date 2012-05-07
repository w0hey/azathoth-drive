#include <EEPROM.h>
#include <TimedAction.h>

#include "pinout.h"
#include "link.h"

// Magic number to verify that *something* is stored in EEPROM
#define EEPROM_MAGIC 0x23
// EEPROM addresses
#define ADDR_MAGIC  0
#define ADDR_X      1
#define ADDR_Y      2

// Default calibration values
const byte x_def = 127;
const byte y_def = 127;

Link link = Link(dispatch_packet);
byte eeprom_x = 0;
byte eeprom_y = 0;
byte x_center = 0;
byte y_center = 0;

// TimedAction "thread" for the safety timer. Set to 1 second by default
TimedAction timeoutAction = TimedAction(1000, timeout);

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
  // TODO: This really needs some way to limit output to a defined range,
  //       as voltages outside 1-4V are detected as a joystick fault.
  char xcmd = packet[3];
  char ycmd = packet[4];
  byte xpos = x_center + xcmd;
  byte ypos = y_center + ycmd;
  analogWrite(P_JOY_X, xpos);
  analogWrite(P_JOY_Y, ypos);
  // reset the safety timer
  timeoutAction.reset();
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
  byte eeprom_magic = EEPROM.read(ADDR_MAGIC);
  if (eeprom_magic == EEPROM_MAGIC) {
    // EEPROM has values stored
    eeprom_x = EEPROM.read(ADDR_X);
    eeprom_y = EEPROM.read(ADDR_Y);
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
  EEPROM.write(ADDR_X, x_center);
  eeprom_x = x_center;
  EEPROM.write(ADDR_Y, y_center);
  eeprom_y = y_center;
  // Store a magic number so we can tell if EEPROM is "empty"
  EEPROM.write(ADDR_MAGIC, EEPROM_MAGIC);
}

void joystickCenter() {
  analogWrite(P_JOY_X, x_center);
  analogWrite(P_JOY_Y, y_center);
}

void timeout() {
  joystickCenter();
}
