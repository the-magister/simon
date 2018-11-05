#ifndef Simon_Common_h
#define Simon_Common_h

//**** Preamble
// we use serialized (bistream'd) structures to communicate, as these
// can be handled by either EasyTransfer (Serial*) and RFM69HW/RFM12b (radio)

//**** Radio

// Band  = 915 MHz
// Group ID = 188
#define D_GROUP_ID 188
// radio nodes/adddresses/ID's
// Simon 1-10
enum nodeID {
  BROADCAST=0, // _everyone_ on group 188
  CONSOLE=1,
  TOWER1=2,
  TOWER2=3,
  TOWER3=4,
  TOWER4=5
};

// Giles 11-20
// Clouds 21-210

//**** Fire

// solenoids take ~50ms to open fully.  Shorter requests are bumped up.
const unsigned long minPropaneTime = 50UL;
// don't leave the solenoids open for longer than this interval.  Longer requests are bumped down.
const unsigned long maxPropaneTime = 2000UL;
// after opening the flame solenoid for N ms, don't reopen for N * propaneClosedMultiplier ms.
const unsigned int propaneClosedMultiplier = 1;

// can add air to the propane for different effects
// by testing, we want to pulse the air
const unsigned long airPulseTime = 50UL;
// by testing, we want to delay introduction of the air
const unsigned long delayAirTime = 50UL;

enum flameEffect {
  // no air.  just straight propane.
  veryRich=0, // "very rich"

  // add some air to 1/3rd of the flame time; not notably different for small (<100ms) flames
  kickStart,  // toss in some air at the beginning
  kickMiddle,  // toss in some air in the middle
  kickEnd,  // toss in some air at the end

  // add air throughout flame time
  gatlingGun, // staccato bursts of air throughout
  randomly, // air tossed in throughout in a random pattern
  veryLean, // as much air as as we can before getting "too lean"

  N_flameEffects  // use this to size arrays appropriately
};

typedef struct {
  byte duration; // flame duration in 10's of ms. e.g. "42" maps to 420 ms.  See min and max constraints.
  byte effect; // see above
} fireInstruction;

// and some definitions, so we're all on the same page
const fireInstruction littlePoof = {minPropaneTime/10, veryRich};
const fireInstruction ragingInferno = {maxPropaneTime/10, randomly};

//***** Colors

// relative locations for the colors on the Simon Console
enum color {
  I_RED=0, // upper right
  I_GRN,   // upper left
  I_BLU,   // lower right
  I_YEL,   // lower left
  N_COLORS, // use this to size arrays appropriately

  I_START = N_COLORS, // 4
  I_RIGHT,
  I_LEFT,
  N_BUTTONS
};

enum nonColorButtons{

};


typedef struct {
  byte red;
  byte green;
  byte blue;
} colorInstruction;

enum animationInstruction {
  A_None,
  A_Clear,
  A_Idle,
  A_Gameplay,
  A_GameplayPressed,
  A_GameplayDecay,
  A_NoRim,
  A_TronCycles,
  A_TronCycles_AddCycle,

  // Strip animations
  A_LaserWipe,
  A_ColorWipe,

  // Rim/matrix animations
  A_ColorWipeMatrix,
  A_ProximityPulseMatrix,

  N_Animations
};

// and some definitions, so we're all on the same page
const colorInstruction cOff = {0, 0, 0};
const colorInstruction cRed = {255, 0, 0};
const colorInstruction cGreen = {0, 255, 0};
const colorInstruction cBlue = {0, 0, 255};
// MGD: Tower does pay attention to this color spec, but Light does not.

//const colorInstruction cYellow = {255, 255, 0};
const colorInstruction cYellow = {255, 100, 0};
const colorInstruction cYellowConsole = {255, 255, 0};
const colorInstruction cWhite = {255, 255, 255};
// and this serves as an easy way to pull out the right RGB color from the
const colorInstruction cMap[N_COLORS] = {cRed, cGreen, cBlue, cYellow};

//**** System Modes

enum systemMode {
  GAMEPLAY=0,
  WHITEOUT,
  BONGO,
  PROXIMITY,
  FIRE,
  LIGHTS,
  LAYOUT,

  EXTERN, // for other projects driving Simon

  N_systemMode //
};

//**** Tie everything together for total system state send

typedef struct {
  byte packetNumber; // track packet number; useful for checking for dropped packets

  byte mode; // what mode are we operating in?

  colorInstruction light[N_COLORS];
  byte animation; // not animationInstruction.  enums are stored as ints (2 bytes), and we only need 1 byte to represent the animations.
  fireInstruction fire[N_COLORS];

} systemState;

#endif
