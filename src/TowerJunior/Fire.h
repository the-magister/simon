// Fire subunit.  Controls propane and air solenoids

#ifndef Fire_h
#define Fire_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers.
#include <Timer.h> // interval timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

class Fire {
  public:
    void begin(byte firePin, byte airPin);
    void update();
    void perform(fireInstruction &inst);

    void stop();

  private:
    // callback for timers; static to drop the implied "this"
    static void airBurst();

    // pin control
    byte firePin, airPin;
    
    // timer control for solenoid impulses
    Timer solenoids;
};

// pin state definitions
#define OFF HIGH
#define ON LOW

#endif

