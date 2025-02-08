#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>


#define BUTTON_SELECT 48
#define BUTTON_DOWN 50
#define BUTTON_UP 52


//#define BSELECT 1
#define BDOWN 1
#define BUP 2

uint8_t readButtons() {
  uint8_t flags=0;
  if (digitalRead(BUTTON_DOWN) == LOW) {
    if(serialOn) Serial.println("Down");
    flags |= BDOWN;
  }
  if (digitalRead(BUTTON_UP) == LOW) {
    if(serialOn) Serial.println("Up");
    flags |= BUP;
  }
  return flags;
}


uint8_t v = 0; //current configuration setting


bool requireUpToStart = false;

/** returns eeprom mem location */
int getMemLocation(int v){
  return (v==BANKMODE ? 0 : BANKSIZE*curBank)+v;
}

void loadConfig(){
  // Read config values from EEPROM
  for (int i = 0; i < NUMCONFIG; i++) {
    config[i] = EEPROM.read(getMemLocation(i));
    if (config[i] == 255 || config[i] > VMAX[i] || config[i] < 0) {
      config[i] = VDEF[curBank][i]; // Set to default value if EEPROM value is invalid
    }
  }
}

void saveConfig(int which=-1){
  byte saved=0;
  for(int v=0;v<NUMCONFIG;v++){
    if(which==-1 || v==which){
      // Save to EEPROM if the current configuration value has changed
      if (EEPROM.read(getMemLocation(v)) != config[v]) {
         ph("Writing to eeprom");
         p("v",v);
         pln("conf",config[v]);
         EEPROM.write(getMemLocation(v), config[v]);
         saved++;
      }
    }
  }
  ///////////////012345678901234567890
  if(saved>1){
    lcd.printRow(3,"Saved changes...");
    delay(250);
  }else if(saved>0){
    ///////////////012345678901234567890
    lcd.printRow(3,"Saved change.");
    delay(250);
  }
  
}



void setupButtons() {
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);
  delay(1);
  loadConfig();
  byte r=readButtons(); //returns 0 to 3
  if(r){
    curBank=r-1; //sets 1 to 2
  }else{
    curBank=config[BANKMODE];
  }
  loadConfig(); //reload due to bankmode change
}

bool selectButton() {
  return digitalRead(BUTTON_SELECT) == LOW;
}

bool upButton() {
  return digitalRead(BUTTON_UP) == LOW;
}

bool downButton() {
  return digitalRead(BUTTON_DOWN) == LOW;
}



void displayConfig(bool header=true) {
  setDevice(0);
  ///////////CONFIGURATION
  ///////////01234567890123456789
  char label[22];
  if(header) {
    sprintf(label,"Config %s mode:",BANKNAMES[curBank]);
    lcd.printRow(2,label);
  }
  strncpy_P(label, VLABELS[v], sizeof(label) - 1); 
  label[20] = '\0'; //null term
  lcd.setCursor(0, 3);
  lcd.print(label);
  if (v != 0) {
    lcd.setCursor(17, 3);
    if(v==BANKMODE){
      lcd.setCursor(15, 3);
      lcd.print(BANKNAMES[config[v]]);
    }else if(v==MINLAPDUR){
      lcd.setCursor(15, 3);
      lcd.print(config[v]*40);
    }else if(VMAX[v]==1){
      lcd.print(config[v]?"On":"Off");
    }else if(VMAX[v]==0){
      //nop
    }else lcd.print(config[v]);
  }
  ph("displayConfig");
  p("label",label);
  pln("cval",(int)config[v]);
}

void displayAllConfig(){
  char buf[40];
             //01234567890123456789
  sprintf(buf,"%s Mode",BANKNAMES[curBank]);
  lcd.printRow(0,buf);
  for(v=0;v<NUMCONFIG;v++){
    displayConfig(false);
    delay(200);
  }
  v=0;
}




void seeChange(bool inc){
   if(v==DEFAULTS){
    if(inc){
      lcd.printRow(0,"Set to defaults...");
      delay(50);
       // Read config values from EEPROM
      for (int i = 0; i < NUMCONFIG; i++) {
          config[i] = VDEF[curBank][i]; // Set to default value 
      }  
      displayAllConfig();
      v=DEFAULTS;
    }
   }else  if(v==BRIGHTNESS){
      setColor(YELLOW);
   }else if(v==RESUME){
     if(inc) {
      saveConfig();
     }else{
      lcd.printRow(3,"Temporary changes...");
     }
     delay(300);
   }else if(v==SOUND){
     playTone(200,100);
   }else if(v==DEMODIAG && inc){    
      lcd.printRow(3,"Reading sensors.");
      delay(50);
      for(int i=0;i<NUMSENSORS;i++){
         lcd.setCursor(0,3);
         sensors[i].display(i);
         delay(300);
      }
      lcd.printRow(3,"Lights demo... ");
      lights.demo();
      delay(1000);
      lcd.printRow(3,"Start song...  ");
      pln("Start song","");
      playF1StartSound1();
      delay(1000);
      lcd.printRow(3,"Restart song...");
      pln("Restart song","");
      playF1Restart();
      delay(1000);
      lcd.printRow(3,"Red flag song...    ");
      pln("Red flag song","");
      playMusic(imperialMarchMelody,imperialMarchNotes,120);
      delay(1000);
      lcd.printRow(3,"Win song...");
      pln("Win song","");
      playMusic(odeToJoyMelody,odeToJoyNotes,80*4);  
   }
}


bool selectPressed=false;

bool configByButtons() {
  if (selectButton()) {
    if(v==BANKMODE){
      saveConfig(v);
    }
    v++; // Increment the selection index
    if (v >= NUMCONFIG) v = 0; // Reset to 0 if it exceeds the number of config variables
    displayConfig(); // Update display with new selection
    while (selectButton()) {} // Wait until the button is released
    delay(300); // Debounce delay
    if (v == 0) requireUpToStart = true;
    else requireUpToStart = false;
    selectPressed=true;
  } else if (upButton() && selectPressed) {
    requireUpToStart = false;
    if (++config[v] > VMAX[v]) config[v] = VMAX[v];
    seeChange(true);
    delay(100); // Debounce delay
    displayConfig(); // Update display with new value
  } else if (downButton()&& selectPressed) {
    requireUpToStart = false;
    if(config[v]>0) config[v]--;
    if (config[v] < 0) config[v] = 0;
    seeChange(false);
    delay(100); // Debounce delay
    displayConfig(); // Update display with new value
  }

//  // For debugging purposes, print the current variable and its value
//  if(serialOn) Serial.print("Config[");
//  if(serialOn) Serial.print(v);
//  if(serialOn) Serial.print("] = ");
//  if(serialOn) Serial.println(config[v]);

  delay(10); // Loop delay to prevent bouncing issues
  if (v != 0 || requireUpToStart) return true; // Inhibit rest of loop, must select back to config 0 to resume
  selectPressed=false;
  return false;
}


//eof
