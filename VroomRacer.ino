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

typedef enum : uint8_t {FORMATION='F', SET='S', RED = 'R', YELLOW = 'Y', GREEN = 'G', CHECKERS = 'C' } RaceFlag;

char raceFlag=FORMATION;

const int LANENUM=2;

//for debugging
#define ph(label) Serial.print(label);Serial.print(':');
#define p(label,var) Serial.print(label); Serial.print(':'); Serial.print(var); Serial.print(',');
#define pln(label,var) Serial.print(label); Serial.print(':'); Serial.print(var); Serial.println();


void mydtostrf(float value, int width,char *buffer) {
  // Calculate the number of digits in the integer part of the value
  int intPart = (int)value;
  if(value>99999 || value<-9999 ){
    sprintf(buffer,"OVER!");
    return;
  }  
  int prec=3;
  if (intPart >= 1000) {
    prec=0;
  } else if (intPart >= 100) {
    prec=1;
  } else if (intPart >= 10) {
    prec=2;
  } else if (intPart >= 1) {
    prec=3;
  }
  if(width<=4 && prec>0) prec--;
  if(width<=3 && prec>0) prec--; //hacky
  // Use dtostrf with the adjusted precision
  dtostrf(value, width, prec, buffer);
}



// include the library code:

#include "MyLCD.h"

// initialize the library with the numbers of the interface pins
MyLCD lcd;
bool sound=true;
volatile int winner=-1;
volatile bool won=false;

int raceLength=5;
unsigned long raceStart=0;




/*

 */

volatile bool raceStarted=false;

#include "Detection.h"
#include "RingBuffer.h"
#include "Ema.h"
#include "Lights.h"
#include "Lane.h"

// Create a Ring Buffer to hold Detection structs
const uint8_t bufferSize = 16; // Must be a power of 2
RingBuffer<Detection, bufferSize> ringBuffer;

#include "MyTone.h"

#include "Sensor.h"
#include "ISR.h"

Lane lanes[LANENUM];

void waveFlag(RaceFlag which){
  for(int i=0;i<LANENUM;i++){
    lanes[i].lights.waveFlag(which);
  }
}


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

#define AVG_CAR_LEN_INCHES 2.5
#define INCHMS_TO_INCHSEC 1000
// CONVERSION=AVG_CAR_LEN_INCHES*INCHMS_TO_INCHSEC
#define CONVERSION 2500


int curPage=0;

long nextPageFlip=0;

void loop() {
   for(int i=0;i<NUMSENSORS;i++){
//      ph(i);
//      sensors[i].debug();    
   }
   loopc++;   
   if(!raceStarted){      
      // Print a message to the LCD.  
      lcd.setCursor(0,1);
      ///////////01234567890123456789
      lcd.print(" Vroom Racer by CB");  
      lcd.setCursor(0,3);
      lcd.print("  Copyright 2024");        
      playF1StartSound1();      
      ISR::calcThresholds();
      raceFlag=GREEN;
      raceStart=millis();
      ISR::go();
      lcd.setCursor(0,2);
      lcd.print("   GO!!!!!!!!");  
      raceStarted=true;
  }
  Detection d; 
  if (ringBuffer.pull(d)) {
    auto i=d.port;
    lanes[i].lights.all(false);
    playTone(400+i*100, 100);
    lanes[i].lights.waveFlag(raceFlag);
    d.debug();        
    p("S#",d.port);
    lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);

    /// Very slow car example
    //10:40:29.574 -> Detection:port:1,value:652,count:17432,timestamp:22000
    //10:40:29.620 -> S#:1,mph:0.15,Sensor:acc:872,lastLapTime:22000,minAcc:866,maxAcc:873,count:0,initialThreshold:652,mainThreshold:435,ticks per ms:18,n:65535

    // 2500*18/17432=2.58 inches/second

    p("inch/sec",lanes[i].speed);
    sensors[d.port].debug();    
    if(lanes[i].lapCounter<raceLength) lanes[i].detect(d);    
    curPage=0;
    nextPageFlip=0;
    if(lanes[i].lapCounter==raceLength){      
      lcd.setCursor(0,i*2);
      if( winner==lanes[i].laneNum ) {
        lcd.print("   WINNER!!!!!!!!");
        lanes[i].lights.green(true);
        playMusic(odeToJoyMelody,odeToJoyNotes,80*4);                        
      }else{                            
        lcd.print("   LOST...");
        lanes[i].lights.red(true);
        playEngine();
      }
    }
  }else{ //STEWARDS
    //check for yellows
    bool anyYellow=false;
    if(!won){
      for(int i=0;i<LANENUM;i++){
        if( lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+lanes[i].avgLapDur*3/2){
          //car is late!
          ph("Yellow detected Car late")
          pln("Car",i);
          anyYellow=true;
        }
      }
    }
    if(anyYellow){
      if(raceFlag!=YELLOW){
        raceFlag=YELLOW;
        waveFlag(raceFlag);
        playMusic(imperialMarchMelody,imperialMarchNotes,120); 
        nextPageFlip=0;    
      }             
    }else if(raceFlag==YELLOW){
      playF1StartSound1();      
      raceFlag=GREEN;
      waveFlag(raceFlag);
      nextPageFlip=0;
    }
  }
  updateLCD();
  delay(100); // Wait before repeating 
}


void updateLCD(){
  if(millis()>nextPageFlip){
    nextPageFlip=millis()+4000;
    for(int i=0;i<LANENUM;i++){
      lanes[i].display(curPage,raceFlag);
    }
    curPage = ++curPage>=PAGECOUNT ? 0:curPage;
  }
}
