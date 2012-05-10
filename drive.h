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

class Drive {
  public:
    Drive();
    void setXCenter(byte);
    void setYCenter(byte);
    void storeCalibration();
    void eraseCalibration();
    void setPosition(char, char);
    void center();
    char* getPosition();
    byte* getRawPosition();
    byte* getCalibration();
    
  private:
    byte eeprom_x;
    byte eeprom_y;
    byte x_center;
    byte y_center;
    byte x_value;
    byte y_value;
    char x_position;
    char y_position;
    boolean readCalibration();
};


#endif
