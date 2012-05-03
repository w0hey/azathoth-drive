// Header file for BB->Arduino datalink

#ifndef _LINK_H_
#define _LINK_H_
#include <Arduino.h>

class Link {
  public:
    Link(void (*function)(int, byte*));
    void service();
    void sendData(int, byte*);
  private:
    byte packet[255];
    byte packet_out[255];
    byte recv;
    int pos;
    byte len;
    boolean xor_next;
    void (*callback)(int, byte*);
    void buildPacket(byte, byte*);
};

#endif
