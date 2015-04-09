// Compile for Mega

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// have: 2K memory, 3 bytes memory per LED, 60 LEDs per meter.
// so, we can support 2000/3/60=11.1 meters, other memory usage notwithstanding.

#include <Adafruit_NeoPixel.h>
// TODO: move to NeoMatrix to take advantage of the 3x stacked strips
#include <Streaming.h>
#include <Metro.h>
#include <EasyTransfer.h>
#include <LightMessage.h> // common message definition

// watchdog timer
#include <avr/wdt.h>

// RIM of LEDs
#define RIM_PIN 3 // wire to rim DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define RIM_N 108*3 // best if divisible by 4
#define RIM_SEG_LENGTH 27 // floor(RIM_N/4)=27
#define YEL_SEG_START 12 // start yellow at this pixel
#define BLU_SEG_START YEL_SEG_START+RIM_SEG_LENGTH
#define RED_SEG_START BLU_SEG_START+RIM_SEG_LENGTH
#define GRN_SEG_START RED_SEG_START+RIM_SEG_LENGTH

// 4x touch lighting strips
#define RED_PIN 4 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define GRN_PIN 5 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define BLU_PIN 6 // wire to button DI pin.  Include a 330 Ohm resistor in series.
#define YEL_PIN 7 // wire to button DI pin.  Include a 330 Ohm resistor in series.
// geometry
#define BUTTON_N 49 // wrapped around each button

// 1x chotsky lighting
#define MIDDLE_PIN 8 //
// geometry
#define MIDDLE_N 18 // wrapped around middle chotsky

// LED indicator to ack button presses
#define LED_PIN 13

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
//create object
EasyTransfer ET;

//give a name to the group of data
LightET lightInst;

// communications with Console module via Serial port
#define LightComms Serial1
#define LIGHT_COMMS_RATE 19200

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// strip around the inner rim
Adafruit_NeoPixel rimJob = Adafruit_NeoPixel(RIM_N, RIM_PIN, NEO_GRB + NEO_KHZ800);

// strips around the buttons
Adafruit_NeoPixel redL = Adafruit_NeoPixel(BUTTON_N, RED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel grnL = Adafruit_NeoPixel(BUTTON_N, GRN_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bluL = Adafruit_NeoPixel(BUTTON_N, BLU_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel yelL = Adafruit_NeoPixel(BUTTON_N, YEL_PIN, NEO_GRB + NEO_KHZ800);

// strip around the middle chotsky
Adafruit_NeoPixel midL = Adafruit_NeoPixel(MIDDLE_N, MIDDLE_PIN, NEO_GRB + NEO_KHZ800);

// LED brightness is not equivalent across colors.  Higher wavelengths are dimmed to balance.
#define RED_MAX 255
#define GRN_MAX 220
#define BLU_MAX 210

#define LED_OFF 0 // makes it a litle brighter in the Console.  0 is fine, too.
// define some colors
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);

// track when we need to send update.
boolean rimUpdated = false;
boolean redUpdated = false;
boolean grnUpdated = false;
boolean bluUpdated = false;
boolean yelUpdated = false;
boolean midUpdated = false;

// reduce intensity at each update, so this is the amount of time we can expect a pixel to last
#define PIXEL_TTL 3000UL

// since the pixels have a TTL, we need to add some when there's nothing going on.
#define STRIP_ADD_PIXEL PIXEL_TTL/3
Metro quietUpdateInterval(STRIP_ADD_PIXEL);

// update the automata on the rim at this interval
#define STRIP_UPDATE 100UL
Metro stripUpdateInterval(STRIP_UPDATE);

// count memory usage for LEDs, which is reported at startup.
#define TOTAL_LED_MEM (RIM_N + BUTTON_N*4 + MIDDLE_N)*3

void setup() {
  Serial.begin(115200);

  delay(500);

  Serial << F("Light startup.") << endl;

  Serial << F("Waiting for Console...") << endl;

  LightComms.begin(LIGHT_COMMS_RATE);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin(details(lightInst), &LightComms);

  // wait for an instruction.
  while( ! ET.receiveData() ) {
    Serial << F(".");
    delay(50);
  }
  Serial << endl;
  sendHandshake();
  
  Serial << F("Console checked in.  Proceeding...") << endl;

  // use WDT to reboot if we hang.
  watchdogSetup();
  Serial << F("Watchdog timer setup complete. 8000 ms reboot time if hung.") << endl;

  Serial << F("Total strip memory usage: ") << TOTAL_LED_MEM << F(" bytes.") << endl;
  Serial << F("Free RAM: ") << freeRam() << endl;

  // random seed from analog noise.
  randomSeed(analogRead(0));

  pinMode(LED_PIN, OUTPUT); digitalWrite(LED_PIN, LOW);

  Serial << F("RimJob: ") << RIM_N << F(" pixels in 4 segments of length ") << RIM_SEG_LENGTH << endl;
  Serial << F("RimJob: YEL starts at: ") << YEL_SEG_START << endl;
  Serial << F("RimJob: BLU starts at: ") << BLU_SEG_START << endl;
  Serial << F("RimJob: RED starts at: ") << RED_SEG_START << endl;
  Serial << F("RimJob: GRN starts at: ") << GRN_SEG_START << endl;

  // Initialize all pixels to 'sweet love makin'
  setupStrip(rimJob, Dead);
  theaterChase(rimJob, SweetLoveMakin, 10);

  setupStrip(midL, Dead);
  theaterChase(midL, SweetLoveMakin, 10);

  setupStrip(redL, Dead);
  theaterChase(redL, Red, 10);
  setupStrip(grnL, Dead);
  theaterChase(grnL, Grn, 10);
  setupStrip(bluL, Dead);
  theaterChase(bluL, Blu, 10);
  setupStrip(yelL, Dead);
  theaterChase(yelL, Yel, 10);
  Serial << F("Free RAM: ") << freeRam() << endl;

  Serial << F("Light: startup complete.") << endl;

  wdt_reset(); // must be called periodically to prevent spurious reboot.
}

void setupStrip(Adafruit_NeoPixel &strip, const uint32_t color) {
  Serial << F("Initializing strip with pixel count: ") << strip.numPixels() << endl;

  strip.begin();

  for ( uint16_t i = 0; i < strip.numPixels(); i++ ) {
    strip.setPixelColor(i, color);
  }

  strip.show();
}

void sendHandshake() {
  // return handshake that we got the instruction.
  byte handShake = 'h';
  LightComms.write(handShake);
//  LightComms.flush(); // wait for xmit to complete.
}

void loop() {
  wdt_reset(); // must be called periodically to prevent spurious reboot.

  // update the strip automata on an interval
  if ( stripUpdateInterval.check() ) {
    // compute the next step and flag for show.
    updateRule90(rimJob, PIXEL_TTL); rimUpdated = true;

    // if we wanted the buttons to animate differently than the rim, this would be the place to do it.
    updateRule90(redL, PIXEL_TTL); redUpdated = true;
    updateRule90(grnL, PIXEL_TTL); grnUpdated = true;
    updateRule90(bluL, PIXEL_TTL); bluUpdated = true;
    updateRule90(yelL, PIXEL_TTL); yelUpdated = true;

    // if we wanted the middle chotsky to animate differently than the rim, this would be the place to do it.
    updateRule90(midL, PIXEL_TTL); midUpdated = true;

    stripUpdateInterval.reset();
  }

  //check and see if a data packet has come in.
  if (ET.receiveData()) {
    quietUpdateInterval.reset();
    sendHandshake(); // heartbeat.

    boolean pressed = false;
    if ( lightInst.red ) {
      buttonPressPattern(0);
      pressed = true;
    }
    if ( lightInst.grn ) {
      buttonPressPattern(1);
      pressed = true;
    }
    if ( lightInst.blu ) {
      buttonPressPattern(2);
      pressed = true;
    }
    if ( lightInst.yel ) {
      buttonPressPattern(3);
      pressed = true;
    }

    if ( pressed ) {
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(LED_PIN, LOW);
    }
  }

  // when it's quiet, add some pixels
  if ( quietUpdateInterval.check() ) {
    quietAddPixels();
    quietUpdateInterval.reset();
  }

  // go to press, if update has occured.
  if ( rimUpdated && rimJob.canShow() ) {
    rimJob.show();
    rimUpdated = false;
  }
  if ( redUpdated && redL.canShow() ) {
    redL.show();
    redUpdated = false;
  }
  if ( grnUpdated && grnL.canShow() ) {
    grnL.show();
    grnUpdated = false;
  }
  if ( bluUpdated && bluL.canShow() ) {
    bluL.show();
    bluUpdated = false;
  }
  if ( yelUpdated && yelL.canShow() ) {
    yelL.show();
    yelUpdated = false;
  }
  if ( midUpdated && midL.canShow() ) {
    midL.show();
    midUpdated = false;
  }

}

// from: http://forum.arduino.cc/index.php?action=dlattach;topic=63651.0;attach=3585
void watchdogSetup() {
  cli();

  wdt_reset();

  // enter watchdog config mode
  WDTCSR |= (1 << WDCE) | (1 << WDE);

  // set watchdog settings; 8000 ms timout.
  WDTCSR = (1 << WDIE) | (1 << WDE) | (1 << WDP3) | (0 << WDP2) | (0 << WDP1) | (1 << WDP0);

  sei();
}

ISR(WDT_vect) {
  Serial << F("WATCHDOG!  Rebooting!") << endl;
  Serial << F("Free RAM: ") << freeRam() << endl;
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void quietAddPixels() {
  switch (random(0, 4)) {
    case 0:
      rimJob.setPixelColor(random(0, RIM_N), Red);
      redL.setPixelColor(random(0, BUTTON_N), Red);
      midL.setPixelColor(random(0, MIDDLE_N), Red);
      redUpdated = true;
      break;
    case 1:
      rimJob.setPixelColor(random(0, RIM_N), Grn);
      grnL.setPixelColor(random(0, BUTTON_N), Grn);
      midL.setPixelColor(random(0, MIDDLE_N), Grn);
      grnUpdated = true;
      break;
    case 2:
      rimJob.setPixelColor(random(0, RIM_N), Blu);
      bluL.setPixelColor(random(0, BUTTON_N), Blu);
      midL.setPixelColor(random(0, MIDDLE_N), Blu);
      bluUpdated = true;
      break;
    case 3:
      rimJob.setPixelColor(random(0, RIM_N), Yel);
      yelL.setPixelColor(random(0, BUTTON_N), Yel);
      midL.setPixelColor(random(0, MIDDLE_N), Yel);
      yelUpdated = true;
      break;
  }
  rimUpdated = true;
  midUpdated = true;

}

void buttonPressPattern(uint8_t button) {

  switch (button) {
    case 0:  // red
      buttonPressToButton(redL, Red);
      redUpdated = true;

      buttonPressToRim(Red, RED_SEG_START, RIM_SEG_LENGTH);
      break;
    case 2:  // blue
      buttonPressToButton(bluL, Blu);
      bluUpdated = true;

      buttonPressToRim(Blu, BLU_SEG_START, RIM_SEG_LENGTH);
      break;
    case 3:  // yellow
      buttonPressToButton(yelL, Yel);
      yelUpdated = true;

      buttonPressToRim(Yel, YEL_SEG_START, RIM_SEG_LENGTH);
      break;
    case 1:  // green
      buttonPressToButton(grnL, Grn);
      grnUpdated = true;

      buttonPressToRim(Grn, GRN_SEG_START, RIM_SEG_LENGTH);
      break;
  }

  // flag that update happened.
  rimUpdated = true;
}

void buttonPressToButton(Adafruit_NeoPixel &strip, const uint32_t color) {


  Serial << F("Button.  Adding pixels to Button color: ");
  printColor(color);

  // clear the segment and lay down this color
  for ( uint16_t i = 0; i < strip.numPixels(); i++ )  {
    strip.setPixelColor(i, color);
  }

}

void buttonPressToRim(const uint32_t color, uint16_t segStart, uint16_t segLength) {

  Serial << F("Button.  Adding pixels to Rim from ") << segStart << F(" to ") << (segStart + segLength - 1) % RIM_N << F(". Color: ");
  printColor(color);

  // clear the segment and lay down this color
  for ( uint16_t i = 1; i < segLength; i++ )  {
    rimJob.setPixelColor((segStart + i) % RIM_N, color);
  }

}

// unpack and pack functions
void extractColor(uint32_t c, uint8_t * cv) {
  cv[0] = (c << 8 ) >> 24;
  cv[1] = (c << 16) >> 24;
  cv[2] = (c << 24) >> 24;
}
uint32_t packColor(uint8_t * cv) {
  return ( rimJob.Color(cv[0], cv[1], cv[2]) );
}
void printColor(uint32_t c) {
  // pull out RGB elements
  uint8_t cv[3];
  extractColor(c, cv);
  Serial << F(" R: ") << cv[0] << F(" G: ") << cv[1] << F(" B: ") << cv[2] << endl;
}

// adjust color with a decrement
uint32_t adjustColor(uint32_t c, unsigned long ttl) {
  // pull out RGB elements
  uint8_t cv[3];
  signed int cvl[3];
  extractColor(c, cv);
  int adj;

  // for each color
  for ( uint8_t i = 0; i < 3; i++ ) {

    // try to smoothly drop the colors
    if ( i == 0 ) adj = RED_MAX / (PIXEL_TTL / STRIP_UPDATE);
    else if ( i == 1 ) adj = GRN_MAX / (PIXEL_TTL / STRIP_UPDATE);
    else adj = BLU_MAX / (PIXEL_TTL / STRIP_UPDATE);

    if ( cv[i] >= adj + LED_OFF ) { // unsiged stuff.  take care.
      cv[i] = cv[i] - adj;
    } else {
      cv[i] = LED_OFF;
    }
    //    Serial << cv[i] << endl;
  }

  //  while(1);
  // repack and return
  return ( packColor(cv) );
}

// merge colors with some kind of XOR
uint32_t mergeColor(uint32_t c1, uint32_t c2) {
  // use bitwise operations to get at packed color vector contents
  const uint8_t bitIndex[3] = {8, 16, 24};
  // store the color vector result
  uint8_t c1v[3], c2v[3], cv[3];

  // get the RGB components.
  extractColor(c1, c1v);
  extractColor(c2, c2v);

  // for each color
  for ( uint8_t i = 0; i < 3; i++ ) {
    // xor by color channel
    if ( c1v[i] > 0 && c2v[i] > 0 ) {
      // alive in both cells, so drop this channel.
      cv[i] = LED_OFF;
    } else {
      // dead in one, so sum them.  unsigned type is doing some funky stuff when >255.
      cv[i] = max(c1v[i], c2v[i]);
    }
  }
  return ( packColor(cv) );
}

// updates the automata using a modified Rule 90
void updateRule90(Adafruit_NeoPixel &strip, unsigned long ttl) {
  // intialize first cell
  uint32_t cs = strip.getPixelColor(0);
  uint32_t ps = strip.getPixelColor(strip.numPixels() - 1);
  uint32_t ns, color;
  // store the color vector result
  uint8_t cv[3];

  // loop to update
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    // get next state
    if (i < strip.numPixels() - 1) {
      ns = strip.getPixelColor(i + 1);
    }
    else { // wrapping back around
      ns = strip.getPixelColor(0);
    }

    // apply Rule 90
    if ( ps == Dead ) {
      color = adjustColor(ns, ttl);
    } else if ( ns == Dead ) {
      color = adjustColor(ps, ttl);
    } else {
      color = mergeColor(adjustColor(ps, ttl), adjustColor(ns, ttl));
    }

    // set the pixel
    strip.setPixelColor(i, color);

    // next iteration setting
    ps = cs;
    cs = ns;
  }
}

void test(Adafruit_NeoPixel &strip) {
  // Some example procedures showing how to display to the pixels:
  colorWipe(strip, strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip, strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip, strip.Color(0, 0, 255), 50); // Blue

  // Send a theater pixel chase in...
  theaterChase(strip, strip.Color(127, 127, 127), 50); // White
  theaterChase(strip, strip.Color(127,   0,   0), 50); // Red
  theaterChase(strip, strip.Color(  0,   0, 127), 50); // Blue

  rainbow(strip, 20);
  rainbowCycle(strip, 20);
  theaterChaseRainbow(strip, 50);
}

// Fill the dots one after the other with a color
void colorWipe(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(Adafruit_NeoPixel &strip, uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(strip, (i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(Adafruit_NeoPixel &strip, uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(strip, ((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(Adafruit_NeoPixel &strip, uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(Adafruit_NeoPixel &strip, uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel(strip, (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(Adafruit_NeoPixel &strip, byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void HSVtoRGB(int hue, int sat, int val, int *colors) {
  int r, g, b, base;

  // hue: 0-359, sat: 0-255, val (lightness): 0-255

  if (sat == 0) { // Achromatic color (gray).
    colors[0] = val;
    colors[1] = val;
    colors[2] = val;
  } else  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60) {
      case 0:
        colors[0] = val;
        colors[1] = (((val - base) * hue) / 60) + base;
        colors[2] = base;
        break;
      case 1:
        colors[0] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        colors[1] = val;
        colors[2] = base;
        break;
      case 2:
        colors[0] = base;
        colors[1] = val;
        colors[2] = (((val - base) * (hue % 60)) / 60) + base;
        break;
      case 3:
        colors[0] = base;
        colors[1] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        colors[2] = val;
        break;
      case 4:
        colors[0] = (((val - base) * (hue % 60)) / 60) + base;
        colors[1] = base;
        colors[2] = val;
        break;
      case 5:
        colors[0] = val;
        colors[1] = base;
        colors[2] = (((val - base) * (60 - (hue % 60))) / 60) + base;
        break;
    }

  }
}


// number, twinkle color, background color, delay
// twinkleRand(5,strip.Color(255,255,255),strip.Color(255, 0, 100),90);

// twinkle random number of pixels
void twinkleRand(Adafruit_NeoPixel &strip, int num, uint32_t c, uint32_t bg) {
  // set background
  //	 stripSet(bg,0);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, bg);
  }
  // for each num
  for (int i = 0; i < num; i++) {
    strip.setPixelColor(random(strip.numPixels()), c);
  }
}

// other options for effects at: http://funkboxing.com/wordpress/wp-content/_postfiles/sk_qLEDFX_POST.ino

