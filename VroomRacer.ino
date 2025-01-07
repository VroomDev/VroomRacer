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

const bool debug=true;
const int NUMLANES=2;
#include "Lights.h"
 
#include "pitches.h"
#define REST 0

typedef enum : char {FORMATION='F', SET='S', REDFLAG = 'R', YELLOWFLAG = 'Y', GREENFLAG = 'G', CHECKERS = 'C' } RaceFlag;


RaceFlag raceFlag=FORMATION;



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

int raceLength=15;
unsigned long raceStart=0;




/*

 */

volatile bool raceStarted=false;

#include "Detection.h"
#include "RingBuffer.h"
#include "Ema.h"

#include "MyTone.h"
#include "Lane.h"

// Create a Ring Buffer to hold Detection structs
const uint8_t bufferSize = 16; // Must be a power of 2
RingBuffer<Detection, bufferSize> ringBuffer;


#include "Sensor.h"
#include "ISR.h"

Lights lights;
Lane lanes[NUMLANES];

void setColor(Color c){
  lights.setColor(c);
}

void waveFlag(RaceFlag which){
  switch(which){
    case FORMATION:
      setColor(BLACK);
    break;
    case SET:
      setColor(WHITE);
    break;
    case REDFLAG:      
      setColor(RED);
    break;
    case YELLOWFLAG:
      setColor(YELLOW);
    break;
    case GREENFLAG:
      setColor(GREEN);
    break;
    case CHECKERS:
      setColor(PURPLE);
    break;
  }
}



void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nStarting");
  
  lanes[0].setup(0);
  lanes[1].setup(1);
  byte buf[NUMLANES]{30,32};
  lights.setup(6,7,8,buf); //MUST AVOID PINS: 9,10 ON MEGA see https://docs.simplefoc.com/choosing_pwm_pins

//  lights.demo();
//  
//  lights.setLanes(false);
//  delay(100);
//  lights.setLanes(false);
//  delay(100);
//  lights.setLane(0,true);
//  delay(1100);
//  lights.setLane(1,true);
//  delay(1100);
//  lights.setLanes(false);

  pinMode(speakerPin, OUTPUT);
  noTone(speakerPin);
      
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  ///////////01234567890123456789
  lcd.print(" Vroom Racer by CB");  
  lcd.setCursor(0,1);
  lcd.print("  Copyright 2024");
  ////////////
  ISR::setup();
  ////////////
  
  //playMusic(imperialMarchMelody,imperialMarchNotes,90); //lower tempo is slower
//  lanes[0].lights.setColor(PURPLE);
//  lanes[1].lights.setColor(ORANGE);

//  // Tone sequence for the starting lights
//  waveFlag(FORMATION);  
//  waveFlag(SET);
//  delay(1300);
//  waveFlag(REDFLAG);
//  delay(1300);
//  waveFlag(YELLOWFLAG);
//  delay(1300);
//  waveFlag(GREENFLAG);
//  delay(1100);  
  
}




int loopc=0;

#define AVG_CAR_LEN_INCHES 2.5
#define INCHMS_TO_INCHSEC 1000
// CONVERSION=AVG_CAR_LEN_INCHES*INCHMS_TO_INCHSEC
#define CONVERSION 2500


int curPage=0;

long nextPageFlip=0;


    /// Very slow car example
    //10:40:29.574 -> Detection:port:1,value:652,count:17432,timestamp:22000
    //10:40:29.620 -> S#:1,mph:0.15,Sensor:acc:872,lastLapTime:22000,minAcc:866,maxAcc:873,count:0,initialThreshold:652,mainThreshold:435,ticks per ms:18,n:65535
    // 2500*18/17432=2.58 inches/second



int getAvgLapCounter(){
  int tot=0;
  for(int i=0;i<NUMLANES;i++){
      tot+=lanes[i].lapCounter;
  }
  return tot/NUMLANES;
}

Detection d; 


void loop() {
   loopc++;   
   if(!raceStarted){      
      // Print a message to the LCD.  
      lcd.setCursor(0,2);
      ///////////01234567890123456789        
      lcd.print("   Get ready. ");
      playF1StartSound1();      
      ISR::calcThresholds();
      raceFlag=GREENFLAG;
      raceStart=millis();
      ISR::go();
      lcd.setCursor(0,3);
      lcd.print("   GO!!!!!!!!");  
      raceStarted=true;
  }
  if (ringBuffer.pull(d)) {
    
    curPage=0;
    nextPageFlip=0;
    auto i=d.port;
    auto aspeed=lanes[i].avgSpeed();
    auto speed=lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);
    if(debug){
      p("S#",d.port);
      sensors[d.port].debug();
      d.debug();
      p("avgSpeed",aspeed);
      p("inch/sec",speed);
    }
    if(lanes[i].lapCounter>=raceLength) { //driver done with race
      alertGoodLap(i);     
    }else if( raceFlag==REDFLAG ){ //lap doesn't count
      alertBadLap(i);     
    }else if(raceFlag==YELLOWFLAG){  // need to make sure going slowly through trap
      if(aspeed==0 || speed<aspeed*3/5) { //lap counts
        alertGoodLap(i); 
      }else{ // too fast!
        alertBadLap(i); 
      }
    }else{ //lap is green
      alertGoodLap(i); 
    }
            
    
        
  }else{ //STEWARDS
    //check for yellows
    bool anyYellow=false,anyRed=false;    
    if(!won){
      auto avgLapCounter=getAvgLapCounter();
      if(avgLapCounter+1==raceLength/2){
        if(raceFlag!=YELLOWFLAG){
            ph("Mid race competition yellow")
        }
        anyYellow=true;
      }else{
        for(int i=0;i<NUMLANES;i++){
          if( lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+(lanes[i].avgLapDur*3)){
            //car is late!
            if(raceFlag!=REDFLAG){
              ph("Red flag detected Car late")
              pln("Car",i);
            }
            anyRed=true;
          }else if( lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+(lanes[i].avgLapDur*3/2)){
            //car is late!
            if(raceFlag!=YELLOWFLAG){
              ph("Yellow detected Car late")
              pln("Car",i);
            }
            anyYellow=true;
          }
        }
      }
    }
    if(anyRed){
      if(raceFlag!=REDFLAG){
        raceFlag=REDFLAG;
        waveFlag(raceFlag);
        playMusic(imperialMarchMelody,imperialMarchNotes,3*120); 
        nextPageFlip=0;    
      }             
    }else if(anyYellow){
      if(raceFlag!=YELLOWFLAG){
        raceFlag=YELLOWFLAG;
        waveFlag(raceFlag);
        playMusic(imperialMarchMelody,imperialMarchNotes,120); 
        nextPageFlip=0;    
      }             
    }else if(raceFlag==YELLOWFLAG || raceFlag==REDFLAG){  //go back to green!      
      raceFlag=GREENFLAG;
      waveFlag(raceFlag);
      playF1StartSound1();      
      nextPageFlip=0;
    }
  }
  updateLCD();
  delay(100); // Wait before repeating 
}


void alertGoodLap(int i) {
  pln("good lap car:",i);
  setColor(PURPLE);
  lights.setLane(i,true);
  playTone(400+i*100, 100);
  if(lanes[i].lapCounter<raceLength) lanes[i].detect(d);    
  if(lanes[i].lapCounter==raceLength){      
    lcd.setCursor(0,i*2+1);
    if( winner==lanes[i].laneNum ) {
      ///////////12345678901234567890
      lcd.print("WINNER!!!          ");
      lights.setColor(GREEN);
      playMusic(odeToJoyMelody,odeToJoyNotes,80*4);                        
    }else{                            
      lcd.print("SORRY...           ");
      lights.setColor(RED);
      playEngine();
    }
  }
  lights.setLane(i,false);
  waveFlag(raceFlag);
}

void alertBadLap(int i){ //,Detection& d){
  pln("bad lap car:",i);
  setColor(ORANGE);
  lights.setLane(i,true);
  playTone(400+i*100, 30);
  lights.setLane(i,false);
  setColor(BLACK);
  delay(30);
  setColor(ORANGE);
  lights.setLane(i,true);
  playTone(400+i*100, 30);
  lights.setLane(i,false);
  waveFlag(raceFlag);
  lanes[i].prior=d; //reset start of this lap
}


void updateLCD(){
  if(millis()>nextPageFlip){
    if(nextPageFlip!=0 && raceFlag==GREENFLAG){
       setColor(BLACK); //don't need that light on all the time    
    }
    nextPageFlip=millis()+4000;
    for(int i=0;i<NUMLANES;i++){
      lanes[i].display(curPage,raceFlag);
    }
    curPage = ++curPage>=PAGECOUNT ? 0:curPage;
  }
}
