#include <EEPROM.h>
#include <TimedAction.h>

#include "pinout.h"
#include "link.h"
#include "drive.h"

#define E_MALLOC 0

Link link = Link(dispatch_packet);
Drive drive = Drive();

// TimedAction "thread" for the safety timer. Set to 1 second by default
TimedAction timeoutAction = TimedAction(1000, timeout);

void setup() {
  // Adjust time 1 for higher frequency PWM
  TCCR1B = TCCR1B & 0b11111000 | 0x01; // 31250 Hz
  pinMode(P_JOY_X, OUTPUT);
  pinMode(P_JOY_Y, OUTPUT);
  Serial.begin(115200);

}

void loop() {
  timeoutAction.check();
  return;
  
}

void serialEvent() {
  link.service();
}

void dispatch_packet(int length, byte* packet) {
  byte len = packet[1] - 1; // we don't need the first payload byte
  byte cmd = packet[2];
  byte *data = (byte*) malloc((len) * sizeof(byte));
  if (data == NULL) {
    handleError(E_MALLOC);
    return;
  }
  memcpy(data, packet + 3, len);
  switch (cmd) {
    case 0x30:
      cmd_joystick(len, data);
      break;
    case 0x40:
      cmd_calibrate(len, data);
      break;
    case 0x41:
      cmd_get_calibration();
      break;
    case 0xF0:
      // soft stop
      drive.center();
      break;
    default:
      break;
  }
  free(data);
}

void cmd_joystick(int length, byte* data) {
  // Expects two signed 8-bit values, indicating the desired joystick
  // position relative to center.
  // TODO: This really needs some way to limit output to a defined range,
  //       as voltages outside 1-4V are detected as a joystick fault.
  char xpos = data[0];
  char ypos = data[1];
  drive.setPosition(xpos, ypos);
  timeoutAction.reset();
  return;
}

void cmd_calibrate(int length, byte* data) {
  switch (data[0]) {
    case 0x00:
      // Set x center
      drive.setXCenter(data[1]);
      break;
    case 0x01:
      // set y center
      drive.setYCenter(data[1]);
      break;
    case 0x10:
      // Write calibration to EEPROM
      drive.storeCalibration();
      break;
  }
}

void cmd_get_calibration() {
  byte* values = drive.getCalibration();
  byte data[5];
  data[0] = 0x41;
  data[1] = values[0];
  data[2] = values[1];
  data[3] = values[2];
  data[4] = values[3];
  link.sendData(5, data);
}

void timeout() {
  drive.center();
}

void handleError(int errcode) {
}
