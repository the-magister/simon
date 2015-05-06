/*******************************************************************************
 * 
 * Bare Conductive MPR121 library
 * ------------------------------
 * 
 * SimpleTouch.ino - simple MPR121 touch detection demo with serial output
 * 
 * Based on code by Jim Lindblom and plenty of inspiration from the Freescale 
 * Semiconductor datasheets and application notes.
 * 
 * Bare Conductive code written by Stefan Dzisiewski-Smith and Peter Krige.
 * 
 * This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 
 * Unported License (CC BY-SA 3.0) http://creativecommons.org/licenses/by-sa/3.0/
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 *******************************************************************************/

#include <MPR121.h>
#include <Wire.h>
#include <Streaming.h>

#define numElectrodes 4

void setup()
{
  Serial.begin(115200);

  Serial.println("Startup.");

  Wire.begin();

  // 0x5C is the MPR121 I2C address on the Bare Touch Board
  if(!MPR121.begin(0x5A)){ 
    Serial.println("error setting up MPR121");  
    parseError();
    while(1);
  }

  // initial data update
  MPR121.updateTouchData();

  Serial.println("Startup complete.");

}

void loop()
{
  // error checking
  if( MPR121.getError() != NO_ERROR ) { 
    parseError(); 
  };

  // check buttons
  for(int i=0; i<numElectrodes; i++){
    if( MPR121.isNewTouch(i) ) Serial << i << " pressed." << endl;
    if( MPR121.isNewRelease(i) ) Serial << i << " released." << endl;
  }

  // update data
  MPR121.updateTouchData();

}

void parseError() {
  switch(MPR121.getError()){
  case NO_ERROR:
    Serial.println("no error");
    break;  
  case ADDRESS_UNKNOWN:
    Serial.println("incorrect address");
    break;
  case READBACK_FAIL:
    Serial.println("readback failure");
    break;
  case OVERCURRENT_FLAG:
    Serial.println("overcurrent on REXT pin");
    break;      
  case OUT_OF_RANGE:
    Serial.println("electrode out of range");
    break;
  case NOT_INITED:
    Serial.println("not initialised");
    break;
  default:
    Serial.println("unknown error");
    break;      
  }
  MPR121.clearError();
}

