// Compile for Uno Pro or Uno Pro Mini

#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Streaming.h> // <<-style printing
#include <Metro.h> // timers

// Serial speed.  match on the other side.
#define SERIAL_SPEED 115200 // baud

// length of time waiting for command before doing stuff.
#define SERIAL_WAIT 5UL // ms.

// character length for commands
#define SERIAL_COMMAND_LENGTH 4 // characters
String command = String(SERIAL_COMMAND_LENGTH);

// minimum volume to set [0-255].  0 is loudest
#define MIN_MP3_VOL 100

// See: https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
// and player_interrupts example

// VS1053 mp3:
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define MP3_CLK 13 // SPI CLK
#define MP3_MISO 12 // SPI MISO
#define MP3_MOSI 11 // SPI MOSI
#define MP3_CS 10 // can move; set to 10 in example; VS1053 chip select pin (output); called BREAKOUT_CS in the example
#define MP3_RST 9 // can move; set to 9 in the example; VS1053 reset pin (output); called BREAKOUT_RESET in the example
#define MP3_XDCS 8 // can move; set to 8 in the example; VS1053 Data/command select pin (output); called BREAKOUT_DCS in the example
#define MP3_SDCS 4 // can move; set to 4 in the example;  Card chip select pin; called CARDCS in the example
#define MP3_DREQ 2 // IRQ 1; set to 3 in the example; VS1053 Data request, ideally an Interrupt pin; called DREQ in the example

// instantiate
Adafruit_VS1053_FilePlayer musicPlayer =
  //  Adafruit_VS1053_FilePlayer(MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ, MP3_SDCS);
  Adafruit_VS1053_FilePlayer( // use hardware SPI with these commented out: MP3_MOSI, MP3_MISO, MP3_CLK,
    MP3_RST, MP3_CS, MP3_XDCS, MP3_DREQ,
    MP3_SDCS);

// holds the number of possible tracks to play in each gesture
int tracksBaff = 0;
int tracksWins = 0;
int tracksLose = 0;
int tracksRock = 0;
// holds the location of the tracks
const char dirBaff[] = "BAFF";
const char dirWins[] = "WINS";
const char dirLose[] = "LOSE";
const char dirRock[] = "ROCK";

void setup() {
  Serial.begin(SERIAL_SPEED);

  // set timeout
  Serial.setTimeout(SERIAL_WAIT);

  // put your setup code here, to run once:
  musicStart();

  // set random seed from analog noise
  randomSeed(analogRead(A0));

}

void loop() {

  // check Serial for commands.
  while ( Serial.available() ) {
    musicPlayer.pausePlaying(true); // hang on.
    command = "";

    while ( command.length() < 4 ) {
      command += Serial.readString(); // beholden to SERIAL_WAIT
    }
    command.trim();

    musicPlayer.pausePlaying(false); // keep going, if apropos.
  }

  if ( command.length() >= 4 )  {
    if ( command.equals(F("UNIT")) ) {
      // unit test
      musicUnitTest();
      Serial << F("Unit test.") << endl;
    } else if ( command.equals(F("PLAY")) || command.equals(F("PAUS")) ) {
      // toggle play/pause
      musicPlayer.pausePlaying(!musicPlayer.paused());
      Serial << F("Toggling play/pause.") << endl;
    } else if ( command.equals(F("STOP")) ) {
      // stop playing
      musicPlayer.stopPlaying();
      Serial << F("Stopping playback.") << endl;
      sendStopSequence(); // and terminate with the stop sequence
    } else if ( command.startsWith(F("VOL")) ) {
      // set volume
      byte vol = command.charAt(SERIAL_COMMAND_LENGTH - 1);
      vol = constrain(map(vol - '0', 1, 9, MIN_MP3_VOL, 0), 0, MIN_MP3_VOL); // 0 is the loudest.  map 1-9 to 0-255.
      // Set volume for left, right channels. lower numbers == louder volume!
      musicPlayer.setVolume(vol, vol);
      Serial << F("Volume set: ") << vol << endl;
    } else if ( command.equals(F("ROCK")) ) {
      playRandomTrack(dirRock, tracksRock);
    } else if ( command.equals(F("WINS")) ) {
      playRandomTrack(dirWins, tracksWins);
    } else if ( command.equals(F("LOSE")) ) {
      playRandomTrack(dirLose, tracksLose);
    } else if ( command.equals(F("BAFF")) ) {
      playRandomTrack(dirBaff, tracksBaff);
    } else {
      Serial << F("Unknown Command(") << command << F(") Try: UNIT, PLAY, PAUS, STOP, VOL[1-9], ROCK, WINS, LOSE, BAFF.") << endl;
    }
  } else if ( command.length() > 0 ) {
    Serial << F("Partial command? :") << command << endl;
    delay(20);
    while ( Serial.read() >= 0 ); // dump buffer.
  }

  // clear
  command = "";

  // store the playing state
  static boolean isPlaying = false;
  // check it, and report if we've stopped
  if ( isPlaying && !musicPlayer.playingMusic) {
    // we've just stopped
    Serial << endl;
    sendStopSequence();
  }
  // store current state
  isPlaying = musicPlayer.playingMusic;

  // put your main code here, to run repeatedly:
  // check if music is running.
  static Metro isPlayingTick(500);
  if (musicPlayer.playingMusic && isPlayingTick.check()) {
    // file is now playing in the 'background' so now's a good time
    // to do something else like handling LEDs or buttons :)
    Serial << F(".");
    isPlayingTick.reset();
  }

}

void sendStopSequence() {
  // mustn't be null terminated.  234=uppercase omega
  Serial.write(234);
}

void playRandomTrack(const char *dirName, int totalTracks) {
  // soft reset.  hate to have this here, but when I flip btw codec types, I get static w/o this.
  musicPlayer.stopPlaying();
  musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);

  // open the directory
  File dir = SD.open(dirName);
  if ( ! dir.isDirectory() ) {
    Serial << F("Error! not a directory: ") << dirName << endl;
  }
  dir.rewindDirectory(); // can't seem to cleanly get the SD card file handlers to start at the top of the dir!!!!

  // pick a track
  int track = random(0, totalTracks);

  // seek file
  File entry = dir.openNextFile(); // have to open a file simply to get it's name!!
  entry.close(); // not actually opening them.  just getting name.

  //  Serial << entry.name() << F("\t");
  for ( int i = 0; i < track; i++ ) {
    entry = dir.openNextFile(dir);
    entry.close();
    //   Serial << entry.name() << F("\t");z
  }
  //  Serial << endl;

  // close up
  dir.close();

  Serial << F(" -> ") << dir.name() << F("/") << entry.name() << F(" [") << track + 1 << F("/") << totalTracks << F("]") << endl;

  // make sure we've stopped.
  while (! musicPlayer.stopped() );

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile(strcat(strcat(dir.name(), "/"), entry.name()))) {
    Serial.println(F("Could not open file."));
    while (1);
  }
}

void musicStart() {
  Serial << F("Startup Adafruit VS1053 breakout.") << endl;
  // initialise the music player
  if (! musicPlayer.begin()) { // initialise the music player
    Serial << F("Couldn't find VS1053, do you have the right pins defined?") << endl;
    while (1);
  }
  Serial << F("VS1053 found") << endl;

  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT))
    Serial << F("DREQ pin is not an interrupt pin.") << endl;

  //  if (! musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT))
  //   Serial << F("Can't use Timer0.") << endl;

  if (!SD.begin(MP3_SDCS)) {
    Serial << F("SD failed, or not present") << endl;
    while (1);  // don't do anything more
  }
  Serial << F("SD OK!") << endl;

  // get track counts
  tracksBaff = countTracks(dirBaff);
  tracksWins = countTracks(dirWins);
  tracksLose = countTracks(dirLose);
  tracksRock = countTracks(dirRock);

  // show counts
  Serial << F("Track counts:") << endl;
  Serial << dirBaff << F(": ") << tracksBaff << endl;
  Serial << dirWins << F(": ") << tracksWins << endl;
  Serial << dirLose << F(": ") << tracksLose << endl;
  Serial << dirRock << F(": ") << tracksRock << endl;

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(MIN_MP3_VOL / 2, MIN_MP3_VOL / 2);
  
  // let the Console know we're not playing.
  sendStopSequence();

}

void musicUnitTest() {
  // soft reset.  hate to have this here, but when I flip btw codec types, I get static w/o this.
  musicPlayer.stopPlaying();
  musicPlayer.sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);

  // Start playing a file, then we can do stuff while waiting for it to finish
  if (! musicPlayer.startPlayingFile("UnitTest.mp3")) {
    Serial.println(F("Could not open file UnitTest.mp3"));
    while (1);
  }
  Serial.println(F("Started playing"));

  /*
  while (musicPlayer.playingMusic) {
     // file is now playing in the 'background' so now's a good time
     // to do something else like handling LEDs or buttons :)
     Serial.print(F("."));
     delay(1000);
   }
   Serial.println(F("Done playing music"));
  */
}

// File listing helper
int countTracks(const char *dirName) {
  int count = 0; // track number

  File dir = SD.open(dirName);

  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      //      Serial << dir.name() << F(" track count: ") << count << endl;
      //     dir.rewindDirectory(); // key!
      break;
    }
    if (!entry.isDirectory()) {
      count ++; // track
    }
    entry.close();
  }

  dir.close();
  return (count);
}


