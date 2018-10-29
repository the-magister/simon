// Lights subunit.  sails lighting.

#ifndef Light_h
#define Light_h

#include <Arduino.h>
#include <FastLED.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // countdown timers

//------ sizes, indexing and inter-unit data structure definitions.
#include <Simon_Common.h>

#define COLOR_ORDER RGB
#define COLOR_CORRECTION TypicalLEDStrip
#define NUM_SAILS 4
#define LEDS_UP 20
#define LEDS_DOWN 20
#define LEDS_SAIL (LEDS_UP+LEDS_DOWN)

// different lighting modes available.
enum lightEffect_t {
  Solid = 0, // always on
  Blink = 1 // blinking with intervals
};

class Light {
  public:

  void begin();
  void update();
  void perform(colorInstruction &inst);
  void effect(lightEffect_t effect = Solid);

  private:

  CRGB currentColor; // lighting

  boolean amBlinking = false;
  const uint16_t onTime = 1000UL;
  const uint16_t offTime = 100UL;
};

#endif

