#include "Fire.h"

// save a pointer to the instatiated Fire class
Fire *thisHack;

void Fire::begin(byte firePin, byte airPin) {
  Serial << F("Fire::begin") << endl;
  
  // save a pointer to this
  thisHack = this;
  // pin assignments
  this->firePin = firePin;
  this->airPin = airPin;
  // order is important.  set then output.
  stop();
  // order is important.  set then output.
  pinMode(firePin, OUTPUT); 
  pinMode(airPin, OUTPUT); 
}

void Fire::update() {
  // run through timer events
  solenoids.update();
}

void Fire::perform(fireInstruction &inst) {
  // track if we're running something
  static boolean isLockedOut = false;
  static Metro lockoutTimer;
  if( lockoutTimer.check() ) isLockedOut = false;
  
  // special cases
  // we're already running a flame effect.  bug out.
  if( isLockedOut ) { 
    Serial << ("Fire: locked out") << endl;
    return;
  }
  // we're not being asked for a flame effect
  if( inst.duration == 0 ) return;
  
  // unpack flame instruction
  unsigned long flameTime = constrain(
    (unsigned long)inst.duration * 10UL, // requested time in ms
    minPropaneTime, maxPropaneTime
  );
  // set the lockout appropriately
  unsigned long lockoutInterval = flameTime*(1UL+propaneClosedMultiplier);
  lockoutTimer.interval(lockoutInterval);
  lockoutTimer.reset();
  
  // reset timers
  this->stop();
  
  // fire it up.
  solenoids.pulseImmediate(firePin, flameTime, ON);
  
  // air effects are more complicated
  switch ( inst.effect ) {
    case veryRich: // just straight propane (DEFAULT) "very rich"
      break; // well, that was easy.

    case kickStart:  // toss in some air at the beginning; can't start for 50ms
      for ( unsigned long i = delayAirTime; i < (flameTime / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case kickMiddle:  // toss in some air in the middle
      for ( unsigned long i = max(delayAirTime, flameTime / 3UL); i < (flameTime * 2UL / 3UL); i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case kickEnd:  // toss in some air at the end
      for ( unsigned long i = max(delayAirTime, flameTime * 2UL / 3UL); i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;

    case gatlingGun: // short bursts of air throughout
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 3UL)
        solenoids.after(i, airBurst); // load timers
      break;
    case randomly:  // toss in some air in a random pattern
      for ( unsigned long i = delayAirTime; i < flameTime; i += random(airPulseTime * 3UL, airPulseTime * 10UL))
        solenoids.after(i, airBurst); // load timers
      break;
    case veryLean: // as much air as as we can before getting "too lean"
      for ( unsigned long i = delayAirTime; i < flameTime; i += airPulseTime * 2UL)
        solenoids.after(i, airBurst); // load timers
      break;
  }

  Serial << F("Fire: effect duration ") << flameTime << F(" ms. Effect ") << inst.effect << F(". Lockout ") << lockoutInterval << endl;
  
  // set fireInstruction back to idle
  inst.duration = 0;
  inst.effect = veryRich;
}

// pulse the air on 
void Fire::airBurst() {
  //  Serial << "Air.   short air:" << millis() << endl;
  thisHack->solenoids.pulseImmediate(thisHack->airPin, airPulseTime, ON);
}

void Fire::stop() {
  digitalWrite(firePin, OFF);
  digitalWrite(airPin, OFF);

  // clear out the timers
  for ( uint8_t i = 0; i < MAX_NUMBER_OF_EVENTS; i++) solenoids.stop(i);
}
