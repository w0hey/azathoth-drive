// class for communicating with the Beagleboard over serial

#include "link.h"
#include <Arduino.h>

Link::Link(void (*function)(int, byte*)) {
  pos = 0;
  len = 0;
  xor_next = false;
  callback = function;
  Serial.begin(115200);
}


// Function to be called when there is pending serial data to process
void Link::service() {
  // FIXME: This looks like an idiot's attempt at a state machine
  while (Serial.available()) {
    // take a byte
    recv = Serial.read();
    
    if (pos == 0) {
      // This should be a start byte, otherwise something is wrong
      if (recv != 0x7e) {
        // ignore it and move on.
        continue;
      }
    }
    
    if (pos == 1) {
      // This is the length field, store it.
      len = recv;
    }
    
    if ((pos != 0) && (recv == 0x7e)) {
      // This is an unexpected start byte, so reset and move on
      len = 0;
      pos = 0;
      xor_next = 0;
    }
    
    // check to see if we need to unescape the next byte
    if (recv == 0x7d) {
      // discard this byte and unescape the next one
      xor_next = true;
      continue;
    }
    
    // check to see if we need to unescape this byte
    if (xor_next) {
      recv = recv ^ 0x20;
      xor_next = false;
    }
    
    // store the byte
    packet[pos] = recv;
    
    // check if we're done with this packet
    if (pos == len + 1) {
      callback(len, packet);
      pos = 0;
      len = 0;
      continue;
    }
    
    // on to the next byte
    pos++;
  }
}

// Takes the provided data, wraps it in a packet header, and sends it.
void Link::sendData(int size, byte data[]) {
  buildPacket(size, data);
  Serial.write(packet_out[0]); // Send start byte, successive bytes must be escaped if necessary
  // skip start byte, add 2 to size to include header
  byte b;
  for (int i = 1; i <= size + 2; i++) {
    b = packet_out[i];
    if (b == 0x7e || b == 0x7d || b == 0x11 || b == 0x13) {
      // byte must be escaped
      b = b ^ 0x20;
      Serial.write(0x7d); // escape marker
    }
    Serial.write(b);
  }
}


// adds a packet header to the provided data for transmission
void Link::buildPacket(byte size, byte data[]) {
  packet_out[0] = 0x7e; // start byte
  packet_out[1] = size; // Length NOT including header
  for (int i = 0; i <= size; i++) {
    packet_out[2 + i] = data[i];
  }
}
