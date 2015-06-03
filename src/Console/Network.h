// Network subunit.  Responsible for interfacing the Console with Towers and other projects

#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
// radio
#include <SPI.h> // radio transmitter is a SPI device
#include <EEPROM.h> // saving and loading radio settings
#include <RFM12B.h> // RFM12b radio transmitter module
#include <QueueArray.h> // queing for IR transmissions

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h> 

#include <RFM12B.h> // Console

// once we get radio comms, wait this long  before returning false from externUpdate.
#define EXTERNAL_COMMS_TIMEOUT 10000UL

// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK/SCK
#define RADIO_SDO 50 // SPI MISO/SDI
#define RADIO_SDI 51 // SPI MOSI/SDO
#define RADIO_SEL 53 // SPI SS/SEL
#define RADIO_IRQ 2 // IRQ 0
#define D_CS_PIN 10 // default SS pin for RFM module

#define CONFIG_SEND_INTERVAL 30000UL // ms. 
#define SEND_INTERVAL 5UL
#define PING_COUNT 10

typedef struct {
  byte address;
  const void * buffer; // Look the fuck out, people.  I assume _you_ keep the instruction in memory long enough for the resend feature to work.
  int size;
  byte sendCount;
} sendBuffer;

class Network {
  public:
    void begin(nodeID node=BROADCAST, unsigned long sendInterval=1UL, byte sendCount=3);
    void update(); // should be called frequently for resends.

    // makes the network do stuff with your stuff
    // NOTE: WE ASSUME THE INSTRUCTIONS REMAIN IN MEMORY (FOR RESEND).
    //  if it's deallocated, we'll send a garbage packet
    void send(colorInstruction &inst, nodeID node=BROADCAST);
    void send(fireInstruction &inst, nodeID node=BROADCAST);
    void send(modeSwitchInstruction &inst, nodeID nodeD=BROADCAST);
    void send(commsCheckInstruction &inst, nodeID node=BROADCAST);
    // and are really just overloaded helpers for this
    void send(const void* buffer, byte bufferSize, nodeID node=BROADCAST);
    
    void ping(int count=10, nodeID node=BROADCAST);
  
  private:
    // gets the network setup
    nodeID networkStart(nodeID node);

    nodeID node; // who am I, really?
    
    // que control for sending
    QueueArray <sendBuffer> que;
    unsigned long sendInterval;
    byte sendCount;
    
    // Need an instance of the Radio Module.  
    RFM12B radio;
};

extern Network network;

#endif
