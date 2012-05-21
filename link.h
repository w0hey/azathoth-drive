// Header file for BB->Mega datalink

#ifndef _LINK_H_
#define _LINK_H_
#include <Arduino.h>

typedef struct {
  byte command;
  void (*handler)(byte, byte*);
} handler_t;

class Link {
  public:
    Link(void(*)(byte));
    void setHandler(byte, void(*)(byte, byte*));
    void service();
    void sendData(int, byte*);
  private:
    byte packet[255]; // buffer for incoming packet
    byte packet_out[255]; // buffer for outgoing packet
    byte recv;
    int pos;
    byte len;
    boolean xor_next;
    
    int nHandlers; // number of registered command handlers
    handler_t* handlers; // array of command handlers
    void (*errHandler)(byte); // pointer to a function to call on error
    
    void buildPacket(byte, byte*);
    void dispatch(byte, byte*);
    void (*getHandler(byte))(byte, byte*);
    
    static int compare_key(const void*, const void*);
    static int compare(const void*, const void*);
};

#endif
