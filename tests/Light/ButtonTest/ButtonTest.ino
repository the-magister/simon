#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition
#include <avr/wdt.h> // watchdog timer
#include "Light.h"
#include "Animations.h"
#include "ConcurrentAnimator.h"
#include "ButtonTest.h"

#define PIN 4

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(49, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix strip = rimJob;

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setStripColor(Adafruit_NeoPixel &strip, int r, int g, int b) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
}

ConcurrentAnimator animator;
AnimationConfig rimConfig;
AnimationConfig redButtonConfig;
AnimationConfig greenButtonConfig;
AnimationConfig blueButtonConfig;
AnimationConfig yellowButtonConfig;
RgbColor red;
RgbColor green;
RgbColor blue;
RgbColor yellow;

AnimateFunc wipeStrip = colorWipe;
AnimateFunc laserStrip = laserWipeEdge;
LaserWipePosition redLaserPos;
LaserWipePosition greenLaserPos;
LaserWipePosition blueLaserPos;
LaserWipePosition yellowLaserPos;

void setup() {

  Serial.begin(115200);

  // Colors
  red.red = RED_MAX;
  red.green = LED_OFF;
  red.blue = LED_OFF;

  yellow.red = RED_MAX;
  yellow.green = GRN_MAX;
  yellow.blue = LED_OFF;

  green.red = LED_OFF;
  green.green = GRN_MAX;
  green.blue = LED_OFF;

  blue.red = LED_OFF;
  blue.green = LED_OFF;
  blue.blue = BLU_MAX;

  // Neopixel strips
  rimJob.begin();
  rimJob.show();

  rimConfig.name = "Outer rim";
  rimConfig.strip = &strip;
  rimConfig.color = red;
  rimConfig.ready = true;
  rimConfig.position = 0;
  rimConfig.timer = Metro(10);

  // Init neo pixel strips for the buttons
  redL.begin();
  redL.show();
  grnL.begin();
  grnL.show();
  bluL.begin();
  bluL.show();
  yelL.begin();
  yelL.show();

  // Red Button
  redButtonConfig.name = "red button";
  redButtonConfig.strip = &redL;
  redButtonConfig.color = red;
  redButtonConfig.ready = true;
  redButtonConfig.position = &redLaserPos;
  redButtonConfig.timer = Metro(1);

  // Green button
  memcpy(&greenButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  greenButtonConfig.name = "green button";
  greenButtonConfig.strip = &grnL;
  greenButtonConfig.color = green;
  greenButtonConfig.position = &greenLaserPos;

  // Blue button
  memcpy(&blueButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  blueButtonConfig.name = "blue button";
  blueButtonConfig.strip = &bluL;
  blueButtonConfig.color = blue;
  blueButtonConfig.position = &blueLaserPos;

  // Yellow button
  memcpy(&yellowButtonConfig, &redButtonConfig, sizeof(AnimationConfig));
  yellowButtonConfig.name = "yellow button";
  yellowButtonConfig.strip = &yelL;
  yellowButtonConfig.color = yellow;
  yellowButtonConfig.position = &yellowLaserPos;
}

/******************************************************************************
 * Main Loop
 ******************************************************************************/
void loop() {
  if (fasterStripUpdateInterval.check()) {
    //animator.animate(wipeStrip, rimConfig);
    animator.animate(laserStrip, redButtonConfig);
    animator.animate(laserStrip, greenButtonConfig);
    animator.animate(laserStrip, blueButtonConfig);
    animator.animate(laserStrip, yellowButtonConfig);

    fasterStripUpdateInterval.reset();
  }
}

void colorWipeMatrix(Adafruit_NeoMatrix &matrix, int c, int wait) {
  for (uint16_t x = 0; x < matrix.width(); x++) {
    matrix.drawPixel(x, 0, c);
    matrix.drawPixel(x, 1, c);
    matrix.drawPixel(x, 2, c);
    matrix.show();
  }
}

// Fill the dots one after the other with a color
// returns the position of the led that was lit
void colorWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  int* pos = (int*) posData;
  int next = (*pos);

  if (next > strip.numPixels()) {
    next = 0;
  } else {
    ++next;
  }

  strip.setPixelColor(next, strip.Color(r, g, b));
  Serial << F("Set pixel: ") << next << endl;
  posData = (void*) next;
}

// color wipes the last 8 pixels
void laserWipe(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  LaserWipePosition* pos = static_cast<LaserWipePosition*>(posData);

  // next is relative to the previous position
  int next = pos->prev;
  int end = strip.numPixels() - 1;
  int start = end - 7;

  // first pixel on, direction set
  if (pos->prev == 0) {
    next = start;
  }

  // reverse direction if at an edge
  if (pos->prev == end) {
    pos->dir = 1;
  }
  else if (pos->prev == start) {
    pos->dir = 0;
  }

  // proceed to the next position
  if (pos->dir == 1) {
    --next;
  } else {
    ++next;
  }

  // clear out the last color and set the next one
  strip.setPixelColor(pos->prev, strip.Color(LED_OFF, LED_OFF, LED_OFF));
  strip.setPixelColor(next, strip.Color(r, g, b));
  pos->prev = next;
}

// color wipes the outside of the rim
void laserWipeEdge(Adafruit_NeoPixel &strip, int r, int g, int b, void *posData) {
  LaserWipePosition* pos = static_cast<LaserWipePosition*>(posData);

  // next is relative to the previous position
  int next = pos->prev;
  int end = end + 31;
  int start = 9;

  /*
  // first pixel on, direction set
  if (pos->prev == 0) {
    next = start;
  }

  // reverse direction if at an edge
  if (pos->prev == end) {
    pos->dir = 1;
  }
  else if (pos->prev == start) {
    pos->dir = 0;
  }

  // proceed to the next position
  if (pos->dir == 1) {
    --next;
  } else {
    ++next;
  }
  */
  if (next == end) {
    next++;
  }
  else if (next > end + 2 || next < end) {
    ++next;
    strip.setPixelColor(pos->prev, strip.Color(LED_OFF, LED_OFF, LED_OFF));
    strip.setPixelColor(next, strip.Color(r, g, b));
    pos->prev = next;
  }

  // clear out the last color and set the next one
}



void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

