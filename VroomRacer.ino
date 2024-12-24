/*
 * Vroom Racer by Chris Busch 
 * (c) 2024
 * 
 * Currently this project isn't broken into a compile+link arragement. 
 * It is kept simple by simply using .h and not all the preprocessor 
 * controls.  That may change in the future if the code base grows.
 */
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
#include "pitches.h"
#define REST 0

//for debugging
#define p(label,var) Serial.print(label); Serial.print(':'); Serial.print(var); Serial.print(',');
#define pln(label,var) Serial.print(label); Serial.print(':'); Serial.print(var); Serial.println();



// include the library code:

#include "MyLCD.h"

// initialize the library with the numbers of the interface pins
MyLCD lcd;
bool sound=true;
volatile int winner=-1;
volatile bool won=false;

int raceLength=15;
unsigned long raceStart=0;


#include "MyTone.h"

/*

 */

volatile bool raceStarted=false;

#include "Detection.h"
#include "RingBuffer.h"
#include "Ema.h"
#include "Lane.h"

// Create a Ring Buffer to hold Detection structs
const uint8_t bufferSize = 16; // Must be a power of 2
RingBuffer<Detection, bufferSize> ringBuffer;

#include "Sensor.h"
#include "ISR.h"


///////////


const int LANENUM=2;
Lane lanes[LANENUM];



void setup() {
  lanes[0].setup(0);
  lanes[1].setup(1);
  pinMode(speakerPin, OUTPUT);
  noTone(speakerPin);
      
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  ////////////
  ISR::setup();
  ////////////
  
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nStarting");
}




int loopc=0;
void loop() {
   loopc++;
   if(raceStarted) lcd.scrollLeft();
   //p("loopc",loopc);
//   p("pings",pings);
//   pln("notReady",notReady);
   if(won){       
   }
   if(!raceStarted){
      // Print a message to the LCD.  
      lcd.setCursor(0,0);
      lcd.print("  Vroom Racer ");  
      lcd.setCursor(0,1);
      lcd.print(" (c) 2024 CGB");  
      //delay(1000);    
      //playF1StartSound();
      playF1StartSound1();
      raceStart=millis();
      ISR::calcThresholds();
      ISR::go();
      lcd.setCursor(0,1);
      lcd.print("   GO!!!!!!!!");  
  }
  raceStarted=true;
  Detection d; 
  if (ringBuffer.pull(d)) {
    auto i=d.port;
    playTone(400+i*100, 100);
    d.debug();    
    p("threshold",sensors[d.port].threshold);
    p("mean",sensors[d.port].mean);
    pln("sd",sensors[d.port].sd);        
    lanes[i].detect(d);    
    lanes[i].display();    
    if(lanes[i].lapCounter==raceLength){      
      lcd.setCursor(0,i);
      if( winner==lanes[i].laneNum ) {
        lcd.print("   WINNER!!!!!!!!");
        playMusic(melody,notes,80*4);                  
      }else{                            
        lcd.print("   LOST...");
        playEngine();
      }
      lanes[i].display(); //update display
    }
  }
  delay(100); // Wait before repeating 
}
