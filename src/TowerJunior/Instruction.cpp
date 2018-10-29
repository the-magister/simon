#include "Instruction.h"

void Instruction::begin(nodeID node) {
  Serial << F("Instruction::begin") << endl;

  this->node = networkStart(node);
  
  this->stateIndex = this->node - TOWER1;
  this->lastPacketNumber = (byte)-1; // 255. wraps.
  
  Serial << F("Instruction: listening to systemState index=") << this->stateIndex << endl;
}

boolean Instruction::update(colorInstruction &colorInst, fireInstruction &fireInst, systemMode &mode) { 
  // check for comms traffic
  if ( radio.receiveDone() ) {
    // process it.
    if ( radio.DATALEN == sizeof(systemState) ) {
      // save instruction
      systemState state;
      state = *(systemState*)radio.DATA;
        
      // copy it out
      colorInst = state.light[this->stateIndex];
      fireInst = state.fire[this->stateIndex];
      mode = (systemMode)state.mode;
      
      // track
      byte packetDelta = state.packetNumber - this->lastPacketNumber; // Wrap!
      if( packetDelta > 1 ) {
        Serial << F("Radio: missed packet.  Last=") << this->lastPacketNumber << F(" Current=") << state.packetNumber << endl;
      } else {
        Serial << F(".");
      }
      this->lastPacketNumber = state.packetNumber;
      
      return( true );
    }
  }
  
  return( false ); // no update
}

// starts the radio
nodeID Instruction::networkStart(nodeID node) {
  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;

  if ( node == BROADCAST ) {
    // try to recover settings from EEPROM
    byte offset = 0;
    byte node = EEPROM.read(radioConfigLocation + (offset++));
    byte groupID = EEPROM.read(radioConfigLocation + (offset++));
    byte band = EEPROM.read(radioConfigLocation + (offset++));

    Serial << F("Tower: Startup RFM69HW radio module. ");
    Serial << F(" NodeID: ") << node;
    Serial << F(" GroupID: ") << groupID;
    Serial << F(" Band: ") << band;
    Serial << endl;

    radio.initialize(band, node, groupID);
    radio.setHighPower(); // for HW boards.
    radio.promiscuous(true); // so broadcasts are received.

    Serial << F("Tower: RFM69HW radio module startup complete. ");

    return( (nodeID)node );
  } else {
    Serial << F("Tower: writing EEPROM (bootstrapping).") << endl;
    // then EEPROM isn't configured correctly.
    byte offset = 0;
    EEPROM.write(radioConfigLocation + (offset++), node);
    EEPROM.write(radioConfigLocation + (offset++), D_GROUP_ID);
    EEPROM.write(radioConfigLocation + (offset++), RF69_915MHZ);

    return ( networkStart(BROADCAST) ); // go again after EEPROM save
  }
}

byte Instruction::getNodeID() {
  return( this->node );
}

