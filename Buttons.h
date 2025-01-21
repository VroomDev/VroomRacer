#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>


#define BUTTON_SELECT 48
#define BUTTON_DOWN 50
#define BUTTON_UP 52


#define BSELECT 1
#define BDOWN 2
#define BUP 3

int readButtons() {
  int flags=0;
  if (digitalRead(BUTTON_SELECT) == LOW) {
    if(serialOn) Serial.println("Select");
    flags |= BSELECT;
  }
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


uint8_t v = 0;


bool requireUpToStart = false;

void setupButtons() {

  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Read config values from EEPROM
  for (int i = 0; i < NUMCONFIG; i++) {
    config[i] = EEPROM.read(i);
    if (config[i] == 255 || config[i] > VMAX[i] || config[i] < 0) {
      config[i] = VDEF[i]; // Set to default value if EEPROM value is invalid
      EEPROM.write(i, config[i]); // Write the default value to EEPROM
    }
  }
  
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
  lcd.setCursor(0, 2);
  ///////////CONFIGURATION
  ///////////01234567890123456789
  if(header) lcd.print("Configuration mode: ");
  char label[22]; 
  strncpy_P(label, VLABELS[v], sizeof(label) - 1); 
  label[20] = '\0'; //null term
  lcd.setCursor(0, 3);
  lcd.print(label);
  if (v != 0) {
    lcd.setCursor(17, 3);
    if(v==MINLAPDUR){
      lcd.setCursor(15, 3);
      lcd.print(config[v]*40);
    }else if(VMAX[v]==1){
      lcd.print(config[v]?"On":"Off");
    }else lcd.print(config[v]);
  }
  ph("displayConfig");
  p("label",label);
  pln("cval",(int)config[v]);
}

void displayAllConfig(){
  for(v=0;v<NUMCONFIG;v++){
    displayConfig(false);
    delay(300);
  }
  v=0;
}

void seeChange(){
   if(v==BRIGHTNESS){
      setColor(YELLOW);
   }else if(v==SOUND){
     playTone(200,100);
   }else if(v==DEMODIAG){
      lcd.print(0,3,"                    ");
      lcd.print(0,3,"Lights demo... ");
      lights.demo();
      delay(1000);
      lcd.print(0,3,"Start song...  ");
      pln("Start song","");
      playF1StartSound1();
      delay(1000);
      lcd.print(0,3,"Restart song...");
      pln("Restart song","");
      playF1Restart();
      delay(1000);
      lcd.print(0,3,"Red flag...    ");
      pln("Red flag song","");
      playMusic(imperialMarchMelody,imperialMarchNotes,120);
      delay(1000);
      lcd.print(0,3,"Win song...    ");
      pln("Win song","");
      playMusic(odeToJoyMelody,odeToJoyNotes,80*4);  
   }
}

bool configByButtons() {
  if (selectButton()) {
    // Save to EEPROM if the current configuration value has changed
    if (EEPROM.read(v) != config[v]) {
       ph("Writing to eeprom");
       p("v",v);
       pln("conf",config[v]);
       EEPROM.write(v, config[v]);
    }
    v++; // Increment the selection index
    if (v >= NUMCONFIG) v = 0; // Reset to 0 if it exceeds the number of config variables
    displayConfig(); // Update display with new selection
    while (selectButton()) {} // Wait until the button is released
    delay(100); // Debounce delay
    if (v == 0) requireUpToStart = true;
    else requireUpToStart = false;
  } else if (upButton()) {
    requireUpToStart = false;
    if (++config[v] > VMAX[v]) config[v] = VMAX[v];
    seeChange();
    delay(100); // Debounce delay
    displayConfig(); // Update display with new value
  } else if (downButton()) {
    requireUpToStart = false;
    if(config[v]>0) config[v]--;
    if (config[v] < 0) config[v] = 0;
    seeChange();
    delay(100); // Debounce delay
    displayConfig(); // Update display with new value
  }

//  // For debugging purposes, print the current variable and its value
//  if(serialOn) Serial.print("Config[");
//  if(serialOn) Serial.print(v);
//  if(serialOn) Serial.print("] = ");
//  if(serialOn) Serial.println(config[v]);

  delay(100); // Loop delay to prevent bouncing issues
  if (v != 0 || requireUpToStart) return true; // Inhibit rest of loop, must select back to config 0 to resume
  return false;
}
