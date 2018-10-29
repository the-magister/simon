// Instructions subunit.  Gets radio traffic and handles idle patterns.

#ifndef Instruction_h
#define Instruction_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <EEPROM.h> // saving and loading radio settings

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

class Instruction {
  public:
    void begin(nodeID node);
    boolean update(colorInstruction &colorInst, fireInstruction &fireInst, systemMode &mode);
    byte getNodeID();
    
  protected:   
    // Need an instance of the Radio Module
    RFM69 radio;
    // store my NODEID
    nodeID node;
    // store the index into systemState
    byte stateIndex;
    // gets the network setup
    nodeID networkStart(nodeID node);
    
    byte lastPacketNumber;
};


#endif

