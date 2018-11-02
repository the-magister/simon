#include "Light.h"

// malloc()
CRGBArray < NUM_SAILS * ( LEDS_UP + LEDS_DOWN ) > Sails;

// references
CRGBSet firstSail = Sails(0*LEDS_SAIL, 1*LEDS_SAIL - 1);
CRGBSet secondSail = Sails(1*LEDS_SAIL, 2*LEDS_SAIL - 1);
CRGBSet thirdSail = Sails(2*LEDS_SAIL, 3*LEDS_SAIL - 1);
CRGBSet fourthSail = Sails(3*LEDS_SAIL, 4*LEDS_SAIL - 1);

// deck lighting
CRGB deckColor = CRGB::FairyLight;

void Light::begin() {
  Serial << F("Light::begin") << endl;
  
  FastLED.addLeds<WS2811, PIN_FASTLED, COLOR_ORDER>(Sails, Sails.size()).setCorrection(COLOR_CORRECTION);

  // set master brightness control
  FastLED.setBrightness(255);
  //  FastLED.setDither( 0 ); // can't do this with WiFi stack?
  
  FastLED.clear();
  FastLED.show();
  delay(1000);
  
}

void Light::effect(lightEffect_t effect) {
  if( effect==Blink ) {
    this->amBlinking = true;
  } else { 
    this->amBlinking = false;
    Sails.fill_solid(this->currentColor);
    FastLED.show();
  }
}

void Light::perform(colorInstruction &inst) {
  // copy out the colors
  this->currentColor.red = inst.red;
  this->currentColor.green = inst.green;
  this->currentColor.blue = inst.blue;
  // do it.
  Sails.fill_solid(this->currentColor);
  FastLED.show();
}

void Light::update() {

  static Metro tick(this->onTime);
  static boolean amOn = false;
  if( this->amBlinking && tick.check() ) {
    amOn = !amOn;
  
    // apply
    if( amOn ) {
      Sails.fill_solid(currentColor);
      tick.interval(this->onTime);
      tick.reset();
    } else {
      Sails.fill_solid(CRGB::Black);
      tick.interval(this->offTime);
      tick.reset();
    } 
    
    FastLED.show();
    
  }
}

