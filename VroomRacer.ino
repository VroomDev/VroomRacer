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


//idea for laptime based fuel: minLapDuration*128/lapDuration

//////////////////////////// CONFIG VALUES
const char* title="VroomRacer v20250719"; // 0526

#define FUELSTEP 64
#define MINLAPDURSTEP 64
#define FRACTIONDENOM 8

typedef enum : uint8_t {RESUME,BANKMODE,BRIGHTNESS,SOUND,RACELEN,FUELING,COMPYELLOW,SPEEDLIMIT,MINLAPDUR,SERIALMONITOR,DEMODIAG,DEFAULTS,
YELLOWDELTA,
REDLAPSNUMER,
NUMCONFIG} Configs; //14 configs

uint8_t config[NUMCONFIG]; 
const char VLABELS[NUMCONFIG][22] PROGMEM =  {
  "- to Exit, + to Save\0", //
  "Boot up mode:       \0",
  "Brightness:         \0", //
  "Sound:              \0", //
  "Race length:        \0", //
  "Fuel(0=off):        \0", //
  "CompYellow Every:   \0", //
  "Yellow Spd limit:   \0", //
  "Min Lap Dur:        \0", //
  "Serial monitor:     \0", //
  "Demo/Diagnostics:   \0", //
  "Revert defaults?    \0", //
  "Yellow millis:      \0", //
  "Red Multiplier+:    \0"
 //01234567890123456789
};
#define NUMBANKS 4
typedef enum : uint8_t {FUELMODE,FASTMODE,TUNEMODE,DRAGMODE} ModeBanks;
const char* BANKNAMES[NUMBANKS]={"Fuel","Fast","Tune","Drag"};
#define BANKSIZE 128
uint8_t curBank=0; 
///////////////////////////////resume, mode, bright,    sound, laps, fuel, compy, limit, minlapdur, sermon, demo, defaults YelDel RedLaps
const uint8_t VMAX[NUMCONFIG]={ 0,  NUMBANKS-1,    8,       1,   99,   254,   99,    254,   254,      1,      1,    0,       254,  254 }; //max vmax is 254
const uint8_t VDEF[NUMBANKS][NUMCONFIG]={
   //R Mode Br snd laps  fuel compyel limit  mLDur             Mon  DD  Def YellowDelta         redlaps
    {0,0,   8, 1,   10,   6,   5,      45,   2500/MINLAPDURSTEP,  1,  0,   0, 2000/MINLAPDURSTEP, 1*FRACTIONDENOM}, //FUEL MODE
    {0,0,   8, 1,   10,   0,   0,      45,   2500/MINLAPDURSTEP,  1,  0,   0, 2000/MINLAPDURSTEP, 1*FRACTIONDENOM}, //Fast MODE
    {0,0,   8, 0,   99,   0,   0,     254,  2500/MINLAPDURSTEP,  1,  0,   0, 0, 0 }, //TUNE MODE
    {0,0,   8, 1,   1,    0,   0,     254,   128/MINLAPDURSTEP,  1,  0,   0, 0, 0 }, //DRAG MODE
};

#define compYellowOn ((bool)config[COMPYELLOW]>0)
#define pitLaneSpeedLimit ((int)config[SPEEDLIMIT])
#define brightness ((int)config[BRIGHTNESS])
#define sound ((bool)config[SOUND])
#define minLapDuration ((int)config[MINLAPDUR]*MINLAPDURSTEP)
#define serialOn ((bool)config[SERIALMONITOR])
//#define serialOn true
#define diagOn ((bool)config[DEMODIAG])
#define fuelOn ((bool)config[FUELING]>0)
#define dragOn (curBank==DRAGMODE)
#define MAXFUEL ((int)config[FUELING]*FUELSTEP)
#define raceLength ((int)config[RACELEN])
#define yellowDelta ((int)config[YELLOWDELTA]*MINLAPDURSTEP)
#define redLapsNumer ((int)config[REDLAPSNUMER])
////////////////////////////END OF CONFIG

const int NUMLANES=2;


// Define an enum to represent different colors
typedef enum  {
  RED,
  GREEN,
  BLUE,
  YELLOW,
  ORANGE,
  PURPLE,
  CYAN,
  WHITE,
  BLACK,
  NUM_COLORS // Keeps track of the number of colors
} Color;

 
#include "pitches.h"
#define REST 0

typedef enum : char {FORMATION='F', SET='S', REDFLAG = 'R', YELLOWFLAG = 'Y', GREENFLAG = 'G', CHECKERS = 'C', DONE='D' } RaceFlag;

//how fast to flip the display page
#define FLIPTIME 15000

RaceFlag raceFlag=FORMATION;


//for debugging
#define ph(label)   if(serialOn){Serial.print(label);Serial.print(':');}
#define p(label,var) if(serialOn){Serial.print(label); Serial.print(':'); Serial.print(var); Serial.print(',');}
#define pln(label,var) if(serialOn){Serial.print(label); Serial.print(':'); Serial.print(var); Serial.println();}


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

#include "Lights.h"


volatile int winner=-1;
volatile bool won=false;

unsigned long raceStart=0;

volatile bool raceStarted=false;


#include "Detection.h"
#include "RingBuffer.h"


#include "MyTone.h"

// Create a Ring Buffer to hold Detection structs
const uint8_t bufferSize = 16; // Must be a power of 2
RingBuffer<Detection, bufferSize> ringBuffer;


#include "Sensor.h"
#include "ISR.h"
#include "Lap.h"
#include "Lane.h"

Lights lights;
Lane lanes[NUMLANES];

void setColor(Color c){
  lights.setColor(c);
}

#include "Buttons.h"


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
  delay(50);
  Serial.println("\n\nStarting");
  if(scanDevices()==0){
    Serial.println("no LCD found");
  }
  // set up the LCD's number of columns and rows:
  for(int d=0;d<nDevices;d++){
    setDevice(d);
    lcd.begin(16, 2);
  }
  //  Serial.println("speaker...");
  pinMode(speakerPin, OUTPUT);
  noTone(speakerPin);

  //  Serial.println("set up buttons...");
  setupButtons(); //NOTE THIS DOES THE CONFIG LOAD AS WELL
  Serial.print("analogRead A0:");
  Serial.println(analogRead(A0));
  delay(10);
  Serial.print("analogRead A1:");
  Serial.println(analogRead(A1));
  delay(10);
  if(dragOn){ //Set up drag racing
      NUMSENSORS=4;
      Serial.println("NUMSENSORS TO 4");
      Serial.print("analogRead A2:");
      Serial.println(analogRead(A2));
      delay(10);
      Serial.print("analogRead A3:");
      Serial.println(analogRead(A3));
      delay(10);
          
  }
  
  //  Serial.println("set up lanes...");
  lanes[0].setup(0);
  lanes[1].setup(1); 
  //  Serial.println("lights...");
  lights.setup(6,7,8); //MUST AVOID PINS: 9,10 ON MEGA see https://docs.simplefoc.com/choosing_pwm_pins
  //setting up display
  for(int d=0;d<nDevices;d++){
    setDevice(d);
    //////     /////01234567890123456789
    lcd.printRow(0,title);  
    lcd.printRow(1,"Copyright 2024 by CB");
    lcd.setCursor(0,2);
    lcd.print(BANKNAMES[curBank]);
    lcd.print(" Mode");
  }
  setDevice(0);
  ////////////
  ISR::setup();
  ////////////
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
bool bannerFlip=false;
unsigned long compYellowStart=0,compYellowStop=0;
Detection d; 
bool needReset=false;
bool lcdDark[MAXSENSORS];


void reset(){
  resetSensors();
  for(int i=0;i<NUMLANES;i++){
     lanes[i].reset();
  }
  winner=-1;
  won=false;
  raceStart=0;
  raceStarted=false;
  curPage=0;
  nextPageFlip=0;
  bannerFlip=false;
  compYellowStart=0;
  compYellowStop=0;
  d.reset(); 
  needReset=false;
  ringBuffer.empty();
}


void fueling(){
  if(fuelOn){
    int s=loopc % NUMSENSORS;
    if(sensors[s].darkEnough && sensors[s].longEnough){
      //in the sensor
      if(lanes[s].fuel<MAXFUEL){
        if( sensors[s].count>250*sensors[s].ticksPerMs) { //in pit
            lanes[s].fuel+=MAXFUEL/20+1;
            if(lanes[s].fuel<MAXFUEL) {
              playTone(400+s*300+lanes[s].fuel, 2); //was playTone ,1
            }else{
              lanes[s].fuel=MAXFUEL;
              dingDing();
            }
            lanes[s].gasBanner();
            nextPageFlip=millis()+500;
        }
      }
    }
  }
}

void loop(){
  loopc++;
  delay(1);
  if(configByButtons()){
    return; //stay in config mode
  }
  if( minusButton()) {
    while(minusButton()){delay(10);} //wait for depress
    nextPageFlip=0;
  }
  if( (loopc & 255)==0 && needReset){
          lcd.setCursor(0,3);
          ///////////01234567890123456789
          lcd.print("Please reset");
          for(int i=0;i<8;i++){
             lcd.print(i<(loopc & 7) ? "." : " ");
          }    
          return; //stop the loop    
  }
  if(curBank==DRAGMODE) dragLoop();
  else raceLoop();
}

bool dragReady=false;



bool verbose=true;
//drag racing logic in here
void dragLoop(){
  if((loopc & 511)==510){
    if(verbose) pln("NUMSENSORS",NUMSENSORS);
  }
  if((loopc & 2047)==0){
    for(int i=0;i<NUMSENSORS;i++){
       if(verbose) p("S#",i);
       if(verbose) sensors[i].debug();
    }
  }else if((loopc & 63)==0){
    for(int i=0;i<NUMSENSORS;i++){
       if(verbose) p(",",sensors[i].acc);
    }
    if(verbose) pln("<--","Sensors");
  }
  if(! dragReady) {
    for(int i=0;i<NUMSENSORS;i++){
      if(sensors[i].n<5000){
       return;
      }
    }
    ISR::calcThresholds();
    dragReady=true;
    ISR::go();
  }else if(! raceStarted ) {
      if(plusButton()){
          startDragRace();    
      }
  }else{
      if( plusButton()) {
         raceStarted=false; //do not have to wait for the other finisher
      }
      if(  ringBuffer.pull(d)) {
        handleDragDetection();
      }     
  }  
  updateDragLCD();
}

void handleDragDetection(){
  nextPageFlip=0;
  d.debug();
  auto i=d.port;
  playToneNoBlock(400+i*300, 50);
  if(d.port<2){ //start sensors
    if(!lanes[i].start.isEmpty()) {
      pln("already started",i);
      return; //ignore this as we already started!
    }      
    lanes[i].start=d;
    auto speed=lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);
    p("port",d.port+1);
    pln("ispeed",speed);
  }else{ //finish sensors
    i-=2;
    if(lanes[i].start.isEmpty()) {
      pln("not crossed start",i);
      return;  //we haven't crossed start
    }
    if(!lanes[i].finish.isEmpty()) {
      pln("we already finished",i);
      return; //ignore this as we already finished!
    }
    playToneNoBlock(400+i*300, 100);
    lanes[i].finish=d;
    auto speed=lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);
    lcd.setCursor(0,0);
    lcd.print("C");
    lcd.print(i+1);
    if(lanes[i].start.timestamp<raceStart) {
      //DQ
      lcd.print(  " is the DQed!!! ");
    }else if(won){
      //didn't win :(
      lcd.print(  " finished.        ");
      //playEngine();
      waveFlag(DONE);
      raceStarted=false;
      curPage=0; 
    }else{
       won=true;
       winner=i;
       ///////////12345678901234567890  
       waveFlag(CHECKERS);
       lcd.print(  " is the WINNER!!! ");
    } 
  }
}


//  01234567890123456789
//0 VroomRacer v20250102
//1 Instructions
//2 C1 R1234 E91234 S123
//3 C2 R1234 E91234 S123

void updateDragLCD(){
  static uint8_t flipper=0;
  if(millis()>nextPageFlip){
    nextPageFlip=millis()+FLIPTIME;
    if(nDevices>0){
      setDevice(0);
      //      if(curPage==0){
      lcd.printRow(0,title);      
      lcd.printRow(1,won ? "Race complete!": (raceStarted?"Go!":"Press + to Drag Race"));
      lanes[++flipper % NUMLANES].display1Drag();
      lanes[++flipper % NUMLANES].display1Drag();
      //      }else{
      //        lanes[++flipper % NUMLANES].displayDrag(curPage);
      //        lanes[++flipper % NUMLANES].displayDrag(curPage);
      //      }
    }
    curPage = ++curPage>=PAGECOUNT ? 0:curPage;
  }
}

void alertDragDetection(int i) {
  if(! lanes[i].detect(d) ){
    alertDragBadDetection(i,lanes[i].why);
    return;
  }
  pln("GOOD LAP car:",i);
  playTone(400+i*300, 100);
  lanes[i].banner0(true,"");
  nextPageFlip=millis()+FLIPTIME/2;
  if(lanes[i].lapCounter==raceLength){      
    lcd.setCursor(0,0);
    lcd.print("C");
    lcd.print(i+1);
    if( winner==lanes[i].laneNum ) {
      ///////////12345678901234567890  
      waveFlag(CHECKERS);
      lcd.print(  " is the WINNER!!! ");
      //playMusic(odeToJoyMelody,odeToJoyNotes,80*4);                        
    }else{                            
      lcd.print(  " finished.        ");
      //playEngine();
      waveFlag(DONE);
      raceStarted=false;
      curPage=0;
    }
  }
  waveFlag(raceFlag);
}


void alertDragBadDetection(int i,char* msg){ //,Detection& d){
  nextPageFlip=millis()+FLIPTIME/2;
  bannerFlip=true;
  p("BAD LAP car:",i);
  pln("msg:",msg);
  lanes[i].fouls++;
  lanes[i].banner(false,msg);
  for(int t=0;t<90;t+=10){
    playTone(400+i*300-t, 20);
  }
  waveFlag(raceFlag);
  lanes[i].prior=d; //reset start of this lap
}


extern volatile unsigned long timer0_millis;

void resetClock(){
//  noInterrupts();
//  timer0_millis=0;
//  interrupts();
}

void startDragRace(){
  resetClock();
  lcd.printRow(0,title);
  lcd.printRow(1,"Get ready.");
  lcd.printRow(2,"");
  lcd.printRow(3,"");
  while(plusButton()){delay(10);} //wait for depress
  reset();
  delay(1000); //debounce
  // Print a message to the LCD.  
  /////     //////01234567890123456789        
  lcd.printRow(1,"Get set.");
  playF1StartSound1();
  raceStart=millis();      
  auto chk=checkSensors();
  if(chk>=0){
      lcd.setCursor(0,2);
      lcd.print("Sensor fault:");
      lcd.print(chk);
      lcd.print(" ");
      lcd.printRow(3,"Please reset.");
      playTone(250,100);
      needReset=true;
      delay(100);         
      return;
  }else{
    raceStarted=true;
  }
  waveFlag(GREENFLAG);
  playTone(1000, 500);  
  ///////////01234567890123456789         
  lcd.printRow(1,"GO!!!!!!!!");
}

void raceLoop(){
  fueling();
  if(diagOn && (loopc & 63)==0){
    for(int i=0;i<NUMSENSORS;i++){
       p("S#",i);
       sensors[i].debug();
    }
  }
   if(!raceStarted){  
      // Print a message to the LCD.  
      lcd.setCursor(0,2);
      ///////////01234567890123456789        
      lcd.print("   Get ready.       ");
      for(int i=0;i<NUMLANES;i++){
        lanes[i].fuel=MAXFUEL;
      }
      playF1StartSound1();      
      ISR::calcThresholds();
      auto chk=checkSensors();
      if(chk>=0){
          lcd.setCursor(0,2);
          lcd.print("Sensor fault Lane:");
          lcd.print(chk);
          lcd.print(" ");
          lcd.printRow(3,"Please reset.");
          playTone(250,100);
          needReset=true;
          delay(100);         
          return;
      }else{
        ISR::go();
        raceStart=millis();
        raceStarted=true;
      }
      waveFlag(GREENFLAG);
      playTone(1000, 500);  
      //  delay(100);
      lcd.setCursor(0,3);
      ///////////01234567890123456789         
      lcd.print("     GO!!!!!!!!     ");    
  }
  if (  ringBuffer.pull(d)) {
    //nextPageFlip=0;
    auto i=d.port;
    auto aspeed=lanes[i].avgSpeed();
    auto speed=lanes[i].setSpeed(CONVERSION*sensors[d.port].ticksPerMs/d.count);
    if(serialOn){
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
      if(aspeed==0 || speed <= pitLaneSpeedLimit || speed<aspeed*3/5) { //lap counts
        alertGoodLap(i); 
      }else{ // too fast!
        alertBadLap(i,"Too fast!"); 
      }
    }else{ //lap is green
      alertGoodLap(i); 
    }
  }
  if((loopc&127)==0){ //STEWARDS
    //check for yellows
    bool anyYellow=false,anyRed=false;    
    if(!won){
      // if we are in a red flag situation, stay red.  Otherwise check to see if in comp yellow period.
      if(compYellowOn && raceFlag!=REDFLAG && millis()>compYellowStart && millis()<compYellowStop){
        if(raceFlag!=YELLOWFLAG){
            ph("Mid race competition yellow")
        }
        anyYellow=true;
      }else{
        for(int i=0;i<NUMLANES;i++){
          //is a car very late? definitely crashed
          //////////////////////////RED CHECK
          if( !(sensors[i].darkEnough || sensors[i].longEnough) //make sure not in pitstop
              && redLapsNumer>0
              && lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+lanes[i].avgLapDur+yellowDelta+(lanes[i].avgLapDur*redLapsNumer/FRACTIONDENOM)){
            //car is late!
            if(raceFlag!=REDFLAG && !anyRed && !anyYellow ){
              p("Red flag detected Car late",(char)raceFlag)
              pln("Car",i);
            }
            anyRed=true;
          //is a car getting late, probably crashed
          //////////////////////////YELLOW CHECK
          }else if( !(sensors[i].darkEnough || sensors[i].longEnough) //make sure not in pitstop
              && yellowDelta>0
              && lanes[i].avgLapDur>0 && millis()>lanes[i].prior.timestamp+lanes[i].avgLapDur+yellowDelta){
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
}


void alertGoodLap(int i) {
  if(! lanes[i].detect(d) ){
    alertBadLap(i,lanes[i].why);
    return;
  }
  p("fuel",lanes[i].fuel);
  pln("GOOD LAP car:",i);
  playTone(400+i*300, 100);
  if(lanes[i].fuel<MAXFUEL/5){
    ph("LOW FUEL");
    delay(50);
    playTone(400+i*300-100, 50);
    delay(50);
    playTone(400+i*300-100, 50);
  }
  lanes[i].banner(true,"");
  nextPageFlip=millis()+FLIPTIME/2;
  bannerFlip=true;
  if(config[COMPYELLOW]>0 
    && ((lanes[i].lapCounter)%(1+config[COMPYELLOW]))==(config[COMPYELLOW]) //happens every comp yellow lap 
    && millis()>compYellowStop){ //only turn it on if the comp yellow is not active
        compYellowStart=millis()+(millis() & 0xFFF); //start in up to 4095ms or 4 seconds random amount
        compYellowStop=compYellowStart+lanes[i].avgLapDur+1000; //make it last for 1 typical lap
  }
  if(lanes[i].lapCounter==raceLength){      
    lcd.setCursor(0,0);
    lcd.print("C");
    lcd.print(i+1);
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
  nextPageFlip=millis()+FLIPTIME/2;
  bannerFlip=true;
  p("BAD LAP car:",i);
  pln("msg:",msg);
  lanes[i].fouls++;
  lanes[i].banner(false,msg);
  for(int t=0;t<90;t+=10){
    playTone(400+i*300-t, 20);
  }
  waveFlag(raceFlag);
  lanes[i].prior=d; //reset start of this lap
}


void updateLCD(){
  static uint8_t flipper=0;
  if(millis()>nextPageFlip){
    nextPageFlip=millis()+FLIPTIME;
    if(bannerFlip && nDevices>1){ //force showing laps
      bannerFlip=false;
      for(int i=0;i<NUMLANES;i++){
         setDevice(i % nDevices);
         lanes[i].banner(true,"",2);
      }
    }else{
      if(nDevices==1){
        lanes[++flipper % NUMLANES].display(curPage,raceFlag);
      }else{
          for(int i=0;i<NUMLANES;i++){
             setDevice(i % nDevices);
             lanes[i].display(curPage,raceFlag);
          }
      }
      curPage = ++curPage>=PAGECOUNT ? 0:curPage;
    }
  }
}


////eof
