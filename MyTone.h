
////////////////////////////////////////////MyTone by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
const int speakerPin = 4; // Pin connected to the speaker


void myTone(int pin,unsigned int frequency){
  if(sound){
    tone(pin,frequency);
  }
  
}
void myTone(int pin,unsigned int frequency,unsigned long duration){
  if(sound) tone(pin,frequency,duration);
}

//blocking play tone
void playTone(int frequency, int duration) {
  myTone(speakerPin, frequency, duration);
  delay(duration);
  noTone(speakerPin);
}


//https://github.com/robsoncouto/arduino-songs
//  Ode to Joy - Beethoven's Symphony No. 9 
// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int odeToJoyMelody[] = {
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
int odeToJoyNotes = sizeof(odeToJoyMelody) / sizeof(odeToJoyMelody[0]) / 2;
  


// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int imperialMarchMelody[] = {
  
  // Dart Vader theme (Imperial March) - Star wars 
  // Score available at https://musescore.com/user/202909/scores/1141521
  // The tenor saxophone part was used
  
//  NOTE_A4,-4, NOTE_A4,-4, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_F4,8, REST,8,
//  NOTE_A4,-4, NOTE_A4,-4, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_A4,16, NOTE_F4,8, REST,8,
//  NOTE_A4,4, NOTE_A4,4, NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16,

//  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,//4
//  NOTE_E5,4, NOTE_E5,4, NOTE_E5,4, NOTE_F5,-8, NOTE_C5,16,
//  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,
  
//  NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, //7 
//  NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16,

//  NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,-16,//9
//  NOTE_C5,4, NOTE_A4,-8, NOTE_C5,16, NOTE_E5,2,
//
//  NOTE_A5,4, NOTE_A4,-8, NOTE_A4,16, NOTE_A5,4, NOTE_GS5,-8, NOTE_G5,16, //7 
//  NOTE_DS5,16, NOTE_D5,16, NOTE_DS5,8, REST,8, 

NOTE_A4,8, NOTE_DS5,4, NOTE_D5,-8, NOTE_CS5,16,
NOTE_C5,16, NOTE_B4,16, NOTE_C5,16, //REST,8, NOTE_F4,8, NOTE_GS4,4, NOTE_F4,-8, NOTE_A4,-16,//9
//  NOTE_A4,4, NOTE_F4,-8, NOTE_C5,16, NOTE_A4,2,
  
};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int imperialMarchNotes = sizeof(imperialMarchMelody) / sizeof(imperialMarchMelody[0]) / 2;


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
  

void playF1StartSound() {
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,1000); delay(500);  noTone(speakerPin); delay(250);
  myTone(speakerPin,2000); delay(1000); noTone(speakerPin);
}

void playF1StartSound1() {
  // Tone sequence for the starting lights
  //waveFlag(FORMATION);
  
  waveFlag(SET);  
  playTone(500, 300);   
  delay(300);
  
  waveFlag(REDFLAG); 
  playTone(500, 300);   
  delay(300);
  
  waveFlag(YELLOWFLAG);
  playTone(500, 300);   
  delay(300);  
  
  waveFlag(GREENFLAG);  
  playTone(1000, 500);  
  delay(100);    
  waveFlag(FORMATION);
}

void playF1Restart() {
  playTone(500, 200);   
  delay(200);
  playTone(500, 200);   
  delay(200);  
  waveFlag(YELLOWFLAG);
  playTone(200, 200);   
  delay(200);  
  waveFlag(GREENFLAG);  
  playTone(1000, 400);  
  delay(100);    
  waveFlag(FORMATION);
}



void playEngine(){
  // Simulating an engine revving up
  for (int i = 500; i <= 1500; i += 100) {
    playTone(i, 100);
  }

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
