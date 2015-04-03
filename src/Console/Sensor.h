// Responsible for sensor readings
#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>

#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers
#include <Bounce.h> // debouncing routine
#include "Sound.h" // for sound module
 
//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

// debounce time
#define SENSOR_DEBOUCE_TIME 100UL

// remote control
#define GAME_ENABLE_PIN A8
  // at system power up, relay is open, meaning pin will read HIGH.
#define GAME_ENABLED HIGH

#define FIRE_ENABLE_PIN A9
  // at system power up, relay is open, meaning pin will read HIGH.
#define FIRE_ENABLED LOW

class Sensor {
  public:
    // startup
    void begin();
    
    // returns true if the sensor indicates an enabled reading
    boolean gameEnabled();
    boolean fireEnabled();
};

extern Sensor sensor;

#endif
