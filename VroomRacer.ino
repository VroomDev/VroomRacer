/*
 * Vroom Racer by Chris Busch 
 * (c) 2024
 * 
 * 
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

void myTone(int pin,unsigned int frequency){
  if(sound){
    tone(pin,frequency);
  }
  
}
void myTone(int pin,unsigned int frequency,unsigned long duration){
  if(sound) tone(pin,frequency,duration);
}

int raceLength=5;
unsigned long raceStart=0;

const int speakerPin = 6; // Pin connected to the speaker
//https://github.com/robsoncouto/arduino-songs
//  Ode to Joy - Beethoven's Symphony No. 9 
// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {
  NOTE_E4,4,  NOTE_E4,4,  NOTE_F4,4,  NOTE_G4,4,//1
  NOTE_G4,4,  NOTE_F4,4,  NOTE_E4,4,  NOTE_D4,4,
  NOTE_C4,4,  NOTE_C4,4,  NOTE_D4,4,  NOTE_E4,4,
  NOTE_E4,-4, NOTE_D4,8,  NOTE_D4,2,

  NOTE_E4,4,  NOTE_E4,4,  NOTE_F4,4,  NOTE_G4,4,//4
  NOTE_G4,4,  NOTE_F4,4,  NOTE_E4,4,  NOTE_D4,4,
  NOTE_C4,4,  NOTE_C4,4,  NOTE_D4,4,  NOTE_E4,4,
  NOTE_D4,-4,  NOTE_C4,8,  NOTE_C4,2,

//  NOTE_D4,4,  NOTE_D4,4,  NOTE_E4,4,  NOTE_C4,4,//8
//  NOTE_D4,4,  NOTE_E4,8,  NOTE_F4,8,  NOTE_E4,4, NOTE_C4,4,
//  NOTE_D4,4,  NOTE_E4,8,  NOTE_F4,8,  NOTE_E4,4, NOTE_D4,4,
//  NOTE_C4,4,  NOTE_D4,4,  NOTE_G3,2,
//
//  NOTE_E4,4,  NOTE_E4,4,  NOTE_F4,4,  NOTE_G4,4,//12
//  NOTE_G4,4,  NOTE_F4,4,  NOTE_E4,4,  NOTE_D4,4,
//  NOTE_C4,4,  NOTE_C4,4,  NOTE_D4,4,  NOTE_E4,4,
//  NOTE_D4,-4,  NOTE_C4,8,  NOTE_C4,2  
};
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
  


void playMusic(int* melody,int notes,int tempo = 80) {
  // this calculates the duration of a whole note in ms
  int wholenote = (60000 * 4) / tempo;
  
  int divider = 0, noteDuration = 0;
 
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    myTone(speakerPin, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration*0.9);

    // stop the waveform generation before the next note.
//    noTone(speakerPin);
  }
  noTone(speakerPin);
}

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
  


/*

 */

volatile bool eyes=false;


const int minLapDuration = 1000; // Debounce delay in milliseconds

const float alpha = 2.0/(500+1); // Smoothing factor (0 < alpha <= 1)  
class Ema {
  public:
    
    volatile float value=-1;

    float learn(float reading){
      if(value<0) value=reading;
      else value = (alpha * reading) + ((1 - alpha) * value);
      return value;      
    }

    float get(){
      return value;
    }
     
};

class Lane {
  public:
    int lightPin = 0;
    volatile int threshold = 500; // Threshold value for detection
    volatile unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
  
    volatile int reading=0;
    volatile int lapCounter=-1;
    int reportedLap=-1;
    volatile unsigned long lapDuration=0,bestLapDur=0,worstLapDur=0;
    volatile Ema sky,shadow; // Variable to store the EMA value 
    volatile bool shadowed=false;
       
    void setup(int pin){
      lightPin=pin;
      lastLapTime=millis();
    }

    /** This returns a value >=0 if lap happened*/
    int reportLap(){
      if(reportedLap<lapCounter){
        reportedLap++;
        return reportedLap;
      }
      return -1;
    }

  void detect(){
    // This is the interrupt handler
    unsigned long currentTime = millis();
    reading = analogRead(lightPin);
    if(!eyes){
      sky.learn(reading);
      return;
    }
    if(!shadowed){ //lit
        threshold=sky.learn(reading)/2;
        // Detect if the value is below the threshold and debounce logic
        if (reading < threshold && (currentTime - lastLapTime > minLapDuration)) { 
          //LAP DETECTED
          shadowed=true;
          if(lastLapTime==0){
            lapDuration=0; //race started crossed finish for first time          
          }else{
            lapDuration = currentTime-lastLapTime;
            if(lapDuration>worstLapDur || worstLapDur==0) worstLapDur=lapDuration;
            if(lapDuration<bestLapDur || bestLapDur==0) bestLapDur=lapDuration;
          }
          lastLapTime = currentTime;
          lapCounter++;
          //lap counter is now the number of completed laps
          if(lapCounter==raceLength){
              if(won){
                //didn't win :( 
              }else{
                winner=lightPin;
                won=true;
              }
          }
          // Car detected, perform your action here
          char fbuffer[10]; // Buffer to store the string representation of the float 
          dtostrf(sky.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
          char buffer[200];
          sprintf(buffer,"%d Car  threshold:%5d sky ema:%10s read:%5d lapCounter:%d lapDuration:%10lu\n",lightPin,threshold,fbuffer,reading,lapCounter,lapDuration);
          Serial.print(buffer);
        }           
    }else{      //shadowed
        shadow.learn(reading)/2;
        threshold=sky.get()/2+shadow.get()/2;
        if(reading>threshold){
          shadowed=false;
          // Car detected, perform your action here
          char fbuffer[10]; // Buffer to store the string representation of the float 
          dtostrf(shadow.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
          char buffer[200];
          sprintf(buffer,"%d LEFT threshold:%5d sha ema:%10s read:%5d lapCounter:%d lapDuration:%10lu\n",lightPin,threshold,fbuffer,reading,lapCounter,lapDuration);
          Serial.print(buffer);          
        }
    }    
  }  

  void display(){
    char buffer[100];
    lcd.setCursor(0,lightPin);//col,row
    char fbuffer[10];
    dtostrf(lapCounter>0 ? ((millis()-raceStart)/lapCounter)/1000.00  : 0, 2, 1, fbuffer); // Convert float to string: width=6, precision=2
    char ch=!won?'C' : winner==lightPin ? 'W' : 'L';    
    sprintf(buffer,"%c%d L%d %4lums %ss           ",ch,lightPin,lapCounter,lapDuration,fbuffer);
    lcd.print(buffer);
  }
 



  
};

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
//      lcd.scrollDisplayLeft();
      scrolled=millis();
//      Serial.print("scroll");
  }  
}

void loop() {
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
          }
       }
    }else{
      scrollLeft(); 
    }
  }  
  delay(100); // Wait 5 seconds before repeating 
}

void playF1StartSound() {
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
//  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,2000); delay(1000); noTone(speakerPin);
}

void playF1StartSound1() {
  // Tone sequence for the starting lights
  playTone(500, 300); delay(300);
  playTone(500, 300); delay(300);
  playTone(500, 300); delay(300);
//  playTone(500, 300); delay(300);
//  playTone(500, 300); delay(300);
  
//  delay(500); // Short pause before the engine sound
  
//  // Simulating an engine revving up
//  for (int i = 500; i <= 1500; i += 100) {
//    playTone(i, 100);
//  }
//  delay(1000); // Pause before the "go" sound
  
  // "Go" sound
  playTone(1000, 500);
}

void playEngine(){
  // Simulating an engine revving up
  for (int i = 500; i <= 1500; i += 100) {
    playTone(i, 100);
  }

}


void playTone(int frequency, int duration) {
  myTone(speakerPin, frequency, duration);
  delay(duration);
  noTone(speakerPin);
}



void sayPhrase() {
  // "pre-"
  playTone(400, 100); delay(150);
  playTone(600, 100); delay(200);
  
  // "-pare"
  playTone(500, 100); delay(100);
  playTone(400, 100); delay(300);
  
  // "to"
  playTone(700, 100); delay(300);
  
  // "qua-"
  playTone(500, 100); delay(150);
  playTone(600, 100); delay(200);
  
  // "-li-"
  playTone(550, 100); delay(100);
  playTone(650, 100); delay(200);
  
  // "-fy"
  playTone(500, 100); delay(150);
  playTone(400, 100);
}



void playCarmen() {
  int melody[] = {
    262, 294, 330, 349, 392, 440, 494, 523,
    523, 494, 440, 392, 349, 330, 294, 262,
    262, 294, 330, 349, 392, 440, 494, 523,
    523, 494, 440, 392, 349, 330, 294, 262
  };

  int noteDurations[] = {
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8
  };

  int numNotes = sizeof(melody) / sizeof(melody[0]);

  for (int i = 0; i < numNotes; i++) {
    int noteDuration = 1000 / noteDurations[i];
    myTone(speakerPin, melody[i], noteDuration);
    delay(noteDuration * 1.30); // Pause between notes
    noTone(speakerPin);
  }
}
