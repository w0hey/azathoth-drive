#ifndef _DRIVE_H_
#define _DRIVE_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "pinout.h"

#define EEPROM_MAGIC 0x23

#define ADDR_MAGIC  0
#define ADDR_X      1
#define ADDR_Y      2
#define X_DEFAULT 127
#define Y_DEFAULT 127

// bitmasks for the status byte
#define STATUS_ESTOP_IN 0x01
#define STATUS_ESTOP_OUT 0x02
#define STATUS_SELECT_IN 0x04
#define STATUS_SELECT_OUT 0x08
#define STATUS_MOVING 0x10

class Drive {
  public:
    Drive(void (*func)());
    void setCenter(byte, byte);
    void storeCalibration();
    void eraseCalibration();
    void setPosition(char, char);
    void select(boolean);
    void estop();
    void reset();
    char* getPosition();
    byte* getRawPosition();
    byte* getCalibration();
    byte getStatus();
    void update();
    
  private:
    byte status;
    byte eeprom_x;
    byte eeprom_y;
    byte x_center;
    byte y_center;
    byte x_value;
    byte y_value;
    char x_position;
    char y_position;
    boolean readCalibration();
    void (*callback)();
};


#endif
