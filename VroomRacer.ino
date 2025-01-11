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

typedef enum : char {FORMATION='F', SET='S', REDFLAG = 'R', YELLOWFLAG = 'Y', GREENFLAG = 'G', CHECKERS = 'C', DONE='D' } RaceFlag;


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
LiquidCrystal_I2C& gLCD=realLcd;

#include "BiggerDigits.h"

bool sound=true;
volatile int winner=-1;
volatile bool won=false;

int raceLength=10;
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
  raceFlag=which;
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
      setColor(CYAN);
    break;
    default: 
      setColor(BLACK);
  }
}



void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("\n\nStarting");
  
  lanes[0].setup(0);
  lanes[1].setup(1);
  byte buf[NUMLANES*2]{30,32,31,33};
  lights.setup(6,7,8,buf); //MUST AVOID PINS: 9,10 ON MEGA see https://docs.simplefoc.com/choosing_pwm_pins

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
 
//  
//  for(int jj=0;jj<NUMLANES;jj++){
//    alertGoodLap(jj);
//    delay(1500);
//    alertBadLap(jj);
//    delay(1500);
//  }
//  alertBadLap(0);
//  delay(1000);
//  alertBadLap(1);
//  delay(1000);
//  //lights.demo();
  //alertBadLap(0,"Too fast!");
}


int loopc=0;

#define AVG_CAR_LEN_INCHES 2.5
#define INCHMS_TO_INCHSEC 1000
// CONVERSION=AVG_CAR_LEN_INCHES*INCHMS_TO_INCHSEC
#define CONVERSION 2500
/// Very slow car example
//10:40:29.574 -> Detection:port:1,value:652,count:17432,timestamp:22000
//10:40:29.620 -> S#:1,mph:0.15,Sensor:acc:872,lastLapTime:22000,minAcc:866,maxAcc:873,count:0,initialThreshold:652,mainThreshold:435,ticks per ms:18,n:65535
// 2500*18/17432=2.58 inches/second


int curPage=0;
long nextPageFlip=0;

//
//int getAvgLapCounter(){
//  int tot=0;
//  int det=0;
//  for(int i=0;i<NUMLANES;i++){
//      tot+=lanes[i].lapCounter;
//      det+=lanes[i].lapCounter>0?1:0;
//  }
//  if(det==0) return 0;
//  return tot/det;
//}
//
//int getMaxLapCounter(){
//  int tot=0;
//  for(int i=0;i<NUMLANES;i++){
//      tot=lanes[i].lapCounter>tot?lanes[i].lapCounter:tot; 
//  }
//  return tot;
//}

unsigned long compYellowStart=0,compYellowStop=0;

Detection d; 




void loop0(){ //demo loop
 const char* m[]={ 
  "Go!  ","Lap 0Car 0",
  "Lap1","     Car 1",
  
  "1.111","Lap11secs ",
   " 999","Lap 1speed",
  
  "Won!","Lap11Car 1",
  "Lost ","Lap11Car 1",
  
  "0.123","Best secs ",
  "11.12","Slow secs ",
  
  "11.12","Avg  secs ",
   " 999","Top  speed",
  
   "  54","Avg  speed",
   "  78","Avg  speed"};

  // Get the size of the array
  int arraySize = sizeof(m) / sizeof(m[0]);
  BigNumber_SendCustomChars();
  // Loop through the array and print each string
  for (int i = 0; i < arraySize; i+=4){
    lanes[0].setSpeed(i);
    lanes[0].lapDuration=i*1000;
    
    drawString(0,0,m[i]);
    printWrap(20-1-4,0,m[i+1]);
    drawString(0,2,m[i+2]);
    printWrap(20-1-4,2,m[i+3]);
    delay(1000);
//    lcd.defineLargeChars();
//    lanes[0].banner(i&1?true:false,m[i]);
//    lanes[0].lapCounter++;
//    delay(1000);
  }
}

void loop() {
   lights.checkFade();
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
      auto chk=checkSensors();
      if(chk>=0){
          lcd.setCursor(0,3);
          lcd.print("Sensor fault Lane:");
          lcd.print(chk);
          playMusic(imperialMarchMelody,imperialMarchNotes,120);
          delay(60000);         
      }else{
        raceStarted=true;
      }
  }
  if (ringBuffer.pull(d)) {
    nextPageFlip=0;
    auto i=d.port;
    auto aspeed=lanes[i].avgSpeed();
    auto speed=lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);
    if(debug){
      pln("-----------------NEW DETECTION-------------","");
      p("C#",d.port);
      sensors[d.port].debug();
      d.debug();
      p("avgSpeed",aspeed);
      pln("inch/sec",speed);
    }
    if(lanes[i].lapCounter>=raceLength) { //driver done with race
      alertGoodLap(i);     
    }else if( raceFlag==REDFLAG ){ //lap doesn't count
      alertBadLap(i,"Red Flag");     
    }else if(raceFlag==YELLOWFLAG){  // need to make sure going slowly through trap
      if(aspeed==0 || speed<30 || speed<aspeed*3/5) { //lap counts
        alertGoodLap(i); 
      }else{ // too fast!
        alertBadLap(i,"Too fast!"); 
      }
    }else{ //lap is green
      alertGoodLap(i); 
    }
  }else{ //STEWARDS
    //check for yellows
    bool anyYellow=false,anyRed=false;    
    if(!won){
      // if we are in a red flag situation, stay red.  Otherwise check to see if in comp yellow period.
      if(raceFlag!=REDFLAG && millis()>compYellowStart && millis()<compYellowStop){
        if(raceFlag!=YELLOWFLAG){
            ph("Mid race competition yellow")
        }
        anyYellow=true;
      }else{
        for(int i=0;i<NUMLANES;i++){
          //is a car very late? definitely crashed
          if( lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+1000+(lanes[i].avgLapDur*3)){
            //car is late!
            if(raceFlag!=REDFLAG && !anyRed && !anyYellow ){
              p("Red flag detected Car late",(char)raceFlag)
              pln("Car",i);
            }
            anyRed=true;
          //is a car getting late, probably crashed
          }else if( lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+500+(lanes[i].avgLapDur*3/2)){
            //car is late!
            if(raceFlag!=YELLOWFLAG && raceFlag!=REDFLAG && !anyRed && !anyYellow){
              p("Yellow detected Car late",(char)raceFlag)
              pln("Car",i);
            }
            anyYellow=true;
          }
        }
      }
    }
    if(anyRed){
      if(raceFlag!=REDFLAG){
        pln("wave","red flag");
        raceFlag=REDFLAG;
        waveFlag(raceFlag);
        playMusic(imperialMarchMelody,imperialMarchNotes,3*120); 
        nextPageFlip=0;    
      }             
    }else if(anyYellow){
      if(raceFlag!=YELLOWFLAG){
        pln("wave","yellow flag");
        raceFlag=YELLOWFLAG;
        waveFlag(raceFlag);
        playMusic(imperialMarchMelody,imperialMarchNotes,120); 
        nextPageFlip=0;    
      }             
    }else if(raceFlag==YELLOWFLAG || raceFlag==REDFLAG){  //go back to green!
      pln("back to","green");            
      playF1Restart();
      raceFlag=GREENFLAG;
      waveFlag(raceFlag);
      nextPageFlip=0;
    }
  }
  updateLCD();
  delay(100); // Wait before repeating 
}


void alertGoodLap(int i) {
  if(! lanes[i].detect(d) ){
    alertBadLap(i,"Too soon!");
    return;
  }
  pln("GOOD LAP car:",i);
  lights.setLane(i,true);
  playTone(400+i*300, 100);
  lanes[i].banner(true,"");
  nextPageFlip=millis()+1000;
  if(lanes[i].lapCounter==raceLength/2 && compYellowStart==0){ //
        compYellowStart=millis()+(millis() & 0xFFF); //start in up to 4 seconds random amount
        compYellowStop=compYellowStart+lanes[i].avgLapDur; //make it last for 1 typical lap
  }
  if(lanes[i].lapCounter==raceLength){      
    lcd.setCursor(0,0);
    lcd.print("C");
    lcd.print(i);
    if( winner==lanes[i].laneNum ) {
      ///////////12345678901234567890  
      waveFlag(CHECKERS);
      lcd.print(  " is the WINNER!!! ");
      playMusic(odeToJoyMelody,odeToJoyNotes,80*4);                        
    }else{                            
      lcd.print(  " finished.        ");
      playEngine();
      waveFlag(DONE);
    }
  }
  waveFlag(raceFlag);
}

void alertBadLap(int i,char* msg){ //,Detection& d){
  nextPageFlip=millis()+1000;
  pln("BAD LAP car:",i);
  lanes[i].banner(false,msg);
  lights.setLane(i,false);
  for(int t=0;t<90;t+=10){
    playTone(400+i*300-t, 20);
  }
  waveFlag(raceFlag);
  lanes[i].prior=d; //reset start of this lap
}


void updateLCD(){
  if(millis()>nextPageFlip){
    if(nextPageFlip!=0 && raceFlag==GREENFLAG){
           
       lights.clearLanes(); 
    }
    nextPageFlip=millis()+4000;
    for(int i=0;i<NUMLANES;i++){
      lanes[i].display(curPage,raceFlag);
    }
    curPage = ++curPage>=PAGECOUNT ? 0:curPage;
  }
}
