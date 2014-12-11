#ifndef Pinouts_h
#define Pinouts_h

// provide some help for initialization functions:

// RFM12b radio:
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define RADIO_SCK 52 // SPI CLK
#define RADIO_SDO 50 // SPI MISO
#define RADIO_SDI 51 // SPI MOSI
#define RADIO_SEL 10 // can move to 8,9,10
#define RADIO_IRQ 2 // IRQ 0

// VS1053 mp3:
// SPI library requirements: http://arduino.cc/en/Reference/SPI
#define MP3_CLK 52 // SPI CLK
#define MP3_MISO 50 // SPI MISO
#define MP3_MOSI 51 // SPI MOSI
#define MP3_CS 40 // can move; set to 10 in example; VS1053 chip select pin (output); called BREAKOUT_CS in the example
#define MP3_RST 41 // can move; set to 9 in the example; VS1053 reset pin (output); called BREAKOUT_RESET in the example
#define MP3_XDCS 42 // can move; set to 8 in the example; VS1053 Data/command select pin (output); called BREAKOUT_DCS in the example
#define MP3_SDCS 43 // can move; set to 4 in the example;  Card chip select pin; called CARDCS in the example
#define MP3_DREQ 3 // IRQ 1; set to 3 in the example; VS1053 Data request, ideally an Interrupt pin; called DREQ in the example

// MPR121 touch:
// Wire library requirements: http://arduino.cc/en/Reference/Wire
#define TOUCH_SCL 21 // Wire SCL
#define TOUCH_SDA 20 // Wire SDA
#define TOUCH_IRQ 19 // IRQ 4, but could move if we don't implement an interrupt

/* damned inconvenient.  will need a bus for these lines:
 +5VDC
 GND
 SPI MOSI 51 
 SPI MISO 50
 SPI SCK 52
*/

// Device list:

/* JeeLibs RFM12b radio comms:
  See: http://jeelabs.org/2011/02/02/meet-the-rfm12b-board/
  
  +5V -> 5V
  Ground -> GND
  SPI clock (SCK) –> Uno digital 13 (Mega 52)
  SPI data out (SDO) –> Uno digital 12 (Mega 50)
  SPI data in (SDI) –> Uno digital 11 (Mega 51)
  SPI select (SEL) –> Uno digital 10
  IRQ –> Uno digital 2
  +3.3V -> (unused, but a regulated 3.3 source if we need it)
*/

/* AdaFruit VS1053 mp3 player:
  See: https://learn.adafruit.com/adafruit-vs1053-mp3-aac-ogg-midi-wav-play-and-record-codec-tutorial/simple-audio-player-wiring
  
  VCC -> 5v
  GND -> GND
  CLK -> Uno #13 (Mega 52)
  MISO -> Uno #12 (Mega 50)
  MOSI -> Uno #11 (Mega 51)
  CS -> Uno #10
  RST -> Uno #9
  XDCS-> Uno #8  
  SDCS -> Uno #4
  DREQ -> Uno #3
  AGND -> lineout Center 'ground' Pin
  LOUT -> lineout Left Pin
  ROUT -> lineout Right Pin
*/

/* AdaFruit MPR121 capsense touch:
  See: https://learn.adafruit.com/adafruit-mpr121-12-key-capacitive-touch-sensor-breakout-tutorial/wiring
  
  Vin -> 5V
  3Vo -> (unused, but a regulated 3.3 source if we need it)
  GND -> GND
  SCL -> Uno A5 (Mega 21)
  SDA -> Uno A4 (Mega 20)
  IRQ -> (unused, but could be added to generate an interrupt on touch and released
  ADDR -> 

    ADDR not connected: 0x5A
    ADDR tied to 3V: 0x5B
    ADDR tied to SDA: 0x5C
    ADDR tied to SCL: 0x5D

*/

/* possible IRQ pins (for attachInterrupt):
  pin 2 (IRQ 0) taken by RFM12b
  pin 3 (IRQ 1) taken by VS1023
  pin 21 (IRQ 2) taken by MPR121 (Wire library!)
  pin 20 (IRQ 3) taken by MPR121 (Wire library!)
  pin 19 (IRQ 4) reserved for MPR121
  pin 18 (IRQ 5)
*/

#endif