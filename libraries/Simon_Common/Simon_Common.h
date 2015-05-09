#ifndef Simon_Common_h
#define Simon_Common_h

// EEPROM location for radio settings.
const byte radioConfigLocation = 42;
// default RFM group
#define D_GROUP_ID 188 

// defined nodes in the network.  
// Console
const byte consoleNodeID = 1;

// how many towers
#define N_TOWERS 4
// Towers
const byte towerNodeID[N_TOWERS] = {2,3,4,5};

// nodeID=0 is understood to be "everyone" (broadcast).

// RFM12b comms
// Band  = 915 MHz
// Group ID = 188
// Simon 1-10
// Giles 11-20
// Clouds 21-210

// how many colors
#define N_COLORS 4
// specific color indices to arrays
#define I_RED 0
#define I_GRN 1
#define I_BLU 2
#define I_YEL 3

// during startup, Console defines which Tower(s) respond to color and fire signals in towerInstruction:
typedef struct {
	// what color(s) should a tower respond to?
	// e.g. lightListen={true, true, true, true} means the tower will show all colors
	// e.g. lightListen={false, true, false, true} means the tower will show green and yellow
	boolean lightListen[N_COLORS]; 
	// what fires(s) should a tower respond to?
	// e.g. as above
	boolean fireListen[N_COLORS];  
	// lighting instructions require no rescaling
	// fire instructions need to rescale [0,255] to [min solenoid opening time, max solenoid opening time]
	// e.g. opening time = map(inst.fireLevel, 0, 255, minFireTime, maxFireTime);
	unsigned long minFireTime; 
	unsigned long maxFireTime; 
	// once the accumulator recloses, don't reopen for a time span which is the prior opening time
	// divided by this number.
	// e.g if the solenoid was just open for 50 ms, it won't open again for 50ms/flameCoolDownDivisor
	unsigned int flameCoolDownDivisor; // enforce a flame shutdown of this interval between poofs
} towerConfiguration;
// EEPROM location for towerConfiguration settings.
const byte towerConfigLocation = 69;

// during gameplay, this is the information passed from Console to Towers:
typedef struct {
	byte lightLevel[N_COLORS]; // 0..255.  maps to analogWrite->light level
	byte fireLevel[N_COLORS]; // 0..255.  maps to timer->fire duration
} towerInstruction;




// during operation, the Console can be reconfigured on-the-fly
typedef struct {
	// how long (ms) between fanfare displays when the system has been idle?
	unsigned long kioskFanfareInterval; 

	// should gameplay let you win with any button press?
	boolean cheatyPantsMode;
	// how many correct moves does Player1 need for each fanfare level?
	byte correctsForFanfare[4];

	// sound levels.  Master [-70 to +4db]
	int masterGain;
	// Tone gain relative to Master
	int toneGainRelativeToMaster;
	// Track gain relative to Tone;
	int trackGainRelativeToTone;
} consoleConfiguration;
// EEPROM location for consoleConfiguration settings.
const byte consoleConfigLocation = 180;

// during operation, the Console can be asked to perform tasks.
// Console will also answer to towerInstructions with the implied setting lightListen={true, true, true, true}.
typedef struct {
	// play some music [1,999].  Set to zero to get a random WINS song; set to -1 to have no Music.
	int playThisTrack;

	// if you ask for Music, do you want the Console to compose light and fire to accompany?
	// if you say "yes"/true, then the Console will send towerInstructions so you shouldn't.
	boolean lightsAndFireWithMusic;

	// do you want button tones to accompany towerInstructions that you send?
	boolean tonesWithTowerInstructions;
} consoleInstruction;

#endif
