/*
 * Vroom Racer by Chris Busch 
 * (c) 2024
 * 
 * Currently this project isn't broken into a compile+link arragement. 
 * It is kept simple by simply using .h and not all the preprocessor 
 * controls.  That may change in the future if the code base grows.
 */

#include "pitches.h"
#define REST 0


// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
bool sound=true;
volatile int winner=-1;
volatile bool won=false;

int raceLength=5;
unsigned long raceStart=0;

const int speakerPin = 6; // Pin connected to the speaker

#include "MyTone.h"

/*

 */

volatile bool eyes=false;


const int minLapDuration = 1000; // Debounce delay in milliseconds

#include "Ema.h"
#include "Lane.h"

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
  // Set up Timer1
  noInterrupts(); // Disable interrupts

  TCCR1A = 0;  // Clear Timer/Counter Control Registers
  TCCR1B = 0;

  TCNT1 = 0; // Initialize counter value to 0
  OCR1A = 15999; // Set compare match register for 1ms intervals (16MHz / 1000 - 1)

  TCCR1B |= (1 << WGM12); // Turn on CTC mode
  TCCR1B |= (1 << CS10) | (1 << CS11); // Set CS10 and CS11 bits for 64 prescaler

  TIMSK1 |= (1 << OCIE1A); // Enable Timer compare interrupt
  interrupts(); // Enable interrupts

  ////////////
  
  Serial.begin(9600);
  Serial.println("\n\nStarting");
}

ISR(TIMER1_COMPA_vect) {  
  for(int i=0;i<LANENUM;i++){
    lanes[i].detect();
  }
  
}



unsigned long scrolled=millis();
void scrollLeft(){
  if( scrolled+1000<millis()){
      lcd.scrollDisplayLeft();
      scrolled=millis();
  }  
}

void loop() {
   if(won){
       scrollLeft();
   }
   if(!eyes){
      // Print a message to the LCD.  
      lcd.setCursor(0,0);
      lcd.print("  Vroom Racer ");  
      lcd.setCursor(0,1);
      lcd.print(" (c) 2024 CGB");  
      //delay(1000);    
      playF1StartSound();
      //delay(1000);
      //playF1StartSound1();
      raceStart=millis();
      lcd.setCursor(0,1);
      lcd.print("   GO!!!!!!!!");  
      scrolled=millis()+2000;
  }
  eyes=true;
  // Generate sound for each syllable in "prepare to qualify"
  for(int i=0;i<LANENUM;i++){
    int laps;
    if((laps=lanes[i].reportLap())>=0){
      //sayPhrase();       
       playTone(400+i*100, 100);
       lanes[i].display();
       scrolled=millis()+2000;
       if(laps==raceLength){
          //playCarmen();
          lcd.setCursor(0,i);
          if( winner==lanes[i].lightPin ) {
            lcd.print("   WINNER!!!!!!!!");
            playMusic(melody,notes,80*4);                  
          }else{                            
            lcd.print("   LOST...         ");
            playEngine();
//            for(int j=0;j<5;j++){
//              lcd.scrollDisplayLeft();
//              delay(100);
//              lcd.scrollDisplayRight();
//              delay(100);
//            }
          }
          lanes[i].display(); //update display
       }
    }else{
//      scrollLeft(); 
    }
  }  
  delay(100); // Wait 5 seconds before repeating 
}
