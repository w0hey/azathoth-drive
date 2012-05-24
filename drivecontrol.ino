#include <EEPROM.h>
#include <TimedAction.h>

#include "pinout.h"
#include "errors.h"
#include "link.h"
#include "drive.h"

enum drivemode {
  LOCAL,
  REMOTE
};

#define RESP_CALIBRATION 0x41
#define RESP_STATUS 0x42
#define RESP_ERROR 0xEE

boolean sendUpdates = true;
drivemode mode = LOCAL;

Link link = Link(handleError);
Drive drive = Drive(driveStateChange);

// TimedAction "thread" for the safety timer. Set to 1 second by default
TimedAction timeoutAction = TimedAction(1000, timeout);
TimedAction driveAction = TimedAction(500, driveUpdate);

void setup() {
  // Adjust timer 1 for higher frequency PWM
  TCCR1B = TCCR1B & 0b11111000 | 0x01; // 31250 Hz
  Serial.begin(115200);
  //driveAction.disable();
  link.setHandler(0x20, mode_handler);
  link.setHandler(0x30, joystick_handler);
  link.setHandler(0x40, calibration_handler);
  link.setHandler(0x41, cal_request_handler);
  link.setHandler(0x43, driveselect_handler);
  link.setHandler(0xfe, reset_handler);
  link.setHandler(0xff, estop_handler);
  byte data[1] = {0x01};
  link.sendData(1, data); // Let the controller know we're here
  timeoutAction.disable(); // disable safety timer until needed
}

void loop() {
  timeoutAction.check();
  driveAction.check();
  return;
  
}

void serialEvent() {
  link.service();
}

void mode_handler(byte length, byte* data) {
  switch (data[0]) {
    case 0x00: // wheelchair mode
      mode = LOCAL;
      drive.center();
      drive.estop();
      drive.select(false);
      drive.reset();
      break;
    case 0x01: // robot mode
      mode = REMOTE;
      drive.center();
      drive.estop();
      drive.select(true);
      drive.reset();
      break;
  }
}

void joystick_handler(byte length, byte* data) {
  // Expects two signed 8-bit values, indicating the desired joystick
  // position relative to center.
  char xpos = data[0];
  char ypos = data[1];
  
  if ((xpos | ypos) != 0) {
    timeoutAction.enable();
  }
  else {timeoutAction.disable();}
  
  drive.setPosition(xpos, ypos);
  timeoutAction.reset();
}

void calibration_handler(byte length, byte* data) {
  switch (data[0]) {
    case 0x00:
      // Set center
      drive.setCenter(data[1], data[2]);
      break;
    case 0x10:
      // Write calibration to EEPROM
      drive.storeCalibration();
      break;
  }
}

void cal_request_handler(byte length, byte* data) {
  byte* values = drive.getCalibration();
  byte payload[5];
  data[0] = RESP_CALIBRATION;
  memcpy(data + 1, values, sizeof(byte) * 4);
  link.sendData(5, data);
}

void driveselect_handler(byte length, byte* data) {
  if (data[0] == 0x01) {
    drive.select(true);
  }
  else {
    drive.select(false);
  }
}

void reset_handler(byte length, byte* data) {
  drive.reset();
}

void estop_handler(byte length, byte* data) {
  drive.estop();
}

void timeout() {
  drive.center();
  handleError(E_TIMEOUT);
}

void driveUpdate() {
  drive.update();
}

void driveStateChange() {
  if (!sendUpdates) {
    return;
  }
  char *pos = drive.getPosition();
  byte *raw = drive.getRawPosition();
  byte status = drive.getStatus();
  byte data[6];
  data[0] = RESP_STATUS;
  data[1] = status;
  data[2] = pos[0];
  data[3] = pos[1];
  data[4] = raw[0];
  data[5] = raw[1];
  link.sendData(6, data);
}

void handleError(byte errcode) {
  byte data[2];
  data[0] = RESP_ERROR;
  data[1] = errcode;
  link.sendData(2, data);
}
