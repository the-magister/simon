#include "Strip.h"

// button pins.  wire to Mega GPIO, bring LOW to indicate pressed.
//create object
EasyTransfer ET;

//give a name to the group of data
systemState inst;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

// strip around the inner rim
//Adafruit_NeoPixel rimJob = Adafruit_NeoPixel(RIM_N, RIM_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoMatrix rimJob = Adafruit_NeoMatrix(
        108, 1, 1, 3, RIM_PIN,
        NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +
        NEO_MATRIX_ROWS +
        NEO_MATRIX_PROGRESSIVE +
        NEO_TILE_BOTTOM + NEO_TILE_LEFT +
        NEO_TILE_ROWS +
        NEO_TILE_PROGRESSIVE,
        NEO_GRB + NEO_KHZ800
        );

// strips around the buttons
Adafruit_NeoPixel redL = Adafruit_NeoPixel(BUTTON_N, RED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel grnL = Adafruit_NeoPixel(BUTTON_N, GRN_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel bluL = Adafruit_NeoPixel(BUTTON_N, BLU_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel yelL = Adafruit_NeoPixel(BUTTON_N, YEL_PIN, NEO_GRB + NEO_KHZ800);

// strip around the middle chotskies
Adafruit_NeoPixel cirL = Adafruit_NeoPixel(CIRCLE_N, CIRCLE_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel placL = Adafruit_NeoPixel(PLACARD_N, PLACARD_PIN, NEO_GRB + NEO_KHZ800);

// define some colors
const uint32_t SweetLoveMakin = rimJob.Color(RED_MAX / 4, GRN_MAX / 6, BLU_MAX / 9);
const uint32_t Red = rimJob.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t Yel = rimJob.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t Grn = rimJob.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t Blu = rimJob.Color(LED_OFF, LED_OFF, BLU_MAX);
const uint32_t Dead = rimJob.Color(LED_OFF, LED_OFF, LED_OFF);
const uint32_t BTN_COLOR_RED = redL.Color(RED_MAX, LED_OFF, LED_OFF);
const uint32_t BTN_COLOR_YELLOW = redL.Color(RED_MAX, GRN_MAX, LED_OFF);
const uint32_t BTN_COLOR_GREEN = redL.Color(LED_OFF, GRN_MAX, LED_OFF);
const uint32_t BTN_COLOR_BLUE = redL.Color(LED_OFF, LED_OFF, BLU_MAX);
// track when we need to send update.
boolean rimUpdated = false;
boolean redUpdated = false;
boolean grnUpdated = false;
boolean bluUpdated = false;
boolean yelUpdated = false;
boolean midUpdated = false;

Metro quietUpdateInterval(STRIP_ADD_PIXEL);
Metro stripUpdateInterval(STRIP_UPDATE);
Metro fasterStripUpdateInterval(STRIP_UPDATE);

// Animations
ConcurrentAnimator animator;
AnimationConfig rimConfig;
AnimationConfig redButtonConfig;
AnimationConfig greenButtonConfig;
AnimationConfig blueButtonConfig;
AnimationConfig yellowButtonConfig;
AnimationConfig circleConfig;
AnimationConfig placardConfig;

// Colors
RgbColor red;
RgbColor green;
RgbColor blue;
RgbColor yellow;

// Animation position configurations
LaserWipePosition redLaserPos;
LaserWipePosition greenLaserPos;
LaserWipePosition blueLaserPos;
LaserWipePosition yellowLaserPos;
int rimPos = 0;
int placPos = 0;
int circPos = 0;

void configureAnimations() {

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

  rimConfig.name = "Outer rim";
  rimConfig.matrix = &rimJob;
  rimConfig.strip = &rimJob;
  rimConfig.color = blue;
  rimConfig.ready = true;
  rimConfig.position = &rimPos;
  rimConfig.timer = Metro(10);

  // Init neo pixel strips for the buttons
  redL.begin();
  grnL.begin();
  bluL.begin();
  yelL.begin();

  cirL.begin();
  placL.begin();

  // Red Button
  redButtonConfig.name = "red button";
  redButtonConfig.strip = &redL;
  redButtonConfig.color = red;
  redButtonConfig.ready = true;
  redButtonConfig.position = &redLaserPos;
  redButtonConfig.timer = Metro(50);

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

  // Circle
  circleConfig.name = "circle";
  circleConfig.strip = &cirL;
  circleConfig.color = yellow;
  circleConfig.ready = true;
  circleConfig.position = &circPos;
  circleConfig.timer = Metro(100);

  // Placard
  placardConfig.name = "placard";
  placardConfig.strip = &placL;
  placardConfig.color = green;
  placardConfig.ready = true;
  placardConfig.position = &placPos;
  placardConfig.timer = Metro(1000);


  // Clear all strips
  setStripColor(redL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(grnL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(bluL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(yelL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(placL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(cirL, LED_OFF, LED_OFF, LED_OFF);
  setStripColor(rimJob, LED_OFF, LED_OFF, LED_OFF);
}

void mapToAnimation(ConcurrentAnimator animator, systemState state) {
    if (state.animation == A_None) {
        Serial << "A_None" << endl;
    }
    if (state.animation == A_LaserWipe) {
        animator.animate(laserWipe, redButtonConfig);
        animator.animate(laserWipe, greenButtonConfig);
        animator.animate(laserWipe, blueButtonConfig);
        animator.animate(laserWipe, yellowButtonConfig);
        Serial << "A_LaserWipe" << endl;
    }
    if (state.animation == A_ColorWipe) {
        animator.animate(colorWipe, placardConfig);
        animator.animate(colorWipe, circleConfig);
        Serial << "A_ColorWipe" << endl;
    }
}

