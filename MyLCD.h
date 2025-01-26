

#define USELCD 1

#ifdef USELCD
#include <Wire.h>



/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */

#include <LiquidCrystal_I2C.h>
#endif

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the parameter is how many columns are on your screen
//the parameter is how many rows are on your screen
#ifdef USELCD
#define MAXLCDS 2
LiquidCrystal_I2C* plcds[MAXLCDS]{NULL}; //pointers to LCDs   //realLcd(0x26, 20, 4);
LiquidCrystal_I2C* curLcd=NULL;
int nDevices;
#endif


//big font code is refactored/based on Hifiduino https://downloadcode.wordpress.com/2009/06/22/code-v-0-7/

#define B 0xFF             // The character for a completely filled box
#define A 0x20             // The character for blank


int scanDevices() {
  byte error, address;
  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 8; address < 120; address++ ) {
    Serial.println(address);
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      curLcd=plcds[nDevices]=new LiquidCrystal_I2C(address, 20, 4);
      nDevices++;
      if(nDevices==MAXLCDS) return nDevices;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } 
  else {
    Serial.println("done");
  }
  return nDevices;
}

void setDevice(int n){
  if(n<nDevices) curLcd=plcds[n];      
}


class MyLCD {


  
  public:

  
  
  // Array index into parts of big numbers. Numbers consist of 9 custom characters in 3 lines
  // 0 1 2 3 4 5 6 7 8 9 
  static const char bn1[];//{B,2,1, 2,1,A, 2,2,1, 2,2,1, 3,A,B, B,2,2, B,2,2, 2,2,B, B,2,1, B,2,1};
  static const char bn2[];//{B,A,B, A,B,A ,3,2,2, A,6,1, 5,6,B, 5,6,7, B,6,7, A,3,2, B,6,B, 5,6,B};
  static const char bn3[];//{4,3,B, 3,B,3, B,3,3, 3,3,B, A,A,B, 3,3,B, 4,3,B, A,B,A, 4,3,B, A,A,B};
    

  // The routine to create the custom characters in the LCD
  void defineLargeChars(){
    // A 1 in the binary representation of the character means it is filled in
    // characters are 5 pixels wide by 8 pixels tall
  
    // We need 7 custom characters for the OPUS DAC display.
    // (Custom character 0 doesn't work in the Web4Robot LCD)
  
    // Define Custom Characters
  
    uint8_t cc1[8] = {     // Custom Character 1
      B11100,
      B11110,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111
    };
  
    uint8_t cc2[8] = {    // Custom Character 2
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B00000,
      B00000,
      B00000
    };
  
    uint8_t cc3[8] = {    // Custom Character 3
      B00000,
      B00000,
      B00000,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111
    };
  
    uint8_t cc4[8] = {   // Custom Character 4
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B01111,
      B00111
    };
  
    uint8_t cc5[8] = {    // Custom Character 5
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B00000,
      B00000
    };
  
    uint8_t cc6[8] = {    // Custom Character 6
      B00000,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111,
      B00000,
      B00000
    };
  
    uint8_t cc7[8] = {     // Custom Character 7
      B00000,
      B11100,
      B11110,
      B11111,
      B11111,
      B11111,
      B11111,
      B11111
    };

    if(curLcd==NULL) return;
  
    // send custom characters to the display
    curLcd->load_custom_character(1,cc1);
    curLcd->load_custom_character(2,cc2);
    curLcd->load_custom_character(3,cc3);
    curLcd->load_custom_character(4,cc4);
    curLcd->load_custom_character(5,cc5);
    curLcd->load_custom_character(6,cc6);
    curLcd->load_custom_character(7,cc7);
  
  }
  



  
  
  void clearLine(){
    if(curLcd==NULL) return;
    #ifdef USELCD
    ///////////////         1         2            3
    ///////////////1234567890123456789012345678901234567890
    curLcd->print("                    ");
    #endif
  }
  
  public:

  int lefts;
  unsigned long scrolled;

  MyLCD():lefts(0),scrolled(millis()){}


  // Routines for LCD Adjustment
  
//  // For LCD backlight adjustment
//  void backLight(uint8_t bright){
//    Wire.beginTransmission(0x4C);
//    Wire.write(0xFE);
//    Wire.write(0x03);
//    Wire.write(bright);
//    Wire.endTransmission();
//    delay(25);
//  }
//  
//  // For LCD contrast adjustment
//  void contrast(uint8_t cont) {
//    Wire.beginTransmission(0x4C);
//    Wire.write(0xFE);
//    Wire.write(0x04);
//    Wire.write(cont);
//    Wire.endTransmission();
//    delay(25);
//  }


  void printRow(int r,const char* msg){
    char buffer[40]="";
    sprintf(buffer,"%-20s",msg);
    buffer[20]=0; //null term
    setCursor(0,r);
    print(buffer);
  }

  void eraseBigDigit(byte col, byte row){
    if(curLcd==NULL) return;
    curLcd->setCursor(col,row+0);
    curLcd->print("    ");
    curLcd->setCursor(col,row+1);
    curLcd->print("    ");
    curLcd->setCursor(col,row+2);
    curLcd->print("    ");
  }

  #define DOTCHAR 'o'

  void printBigDigit(uint8_t digit,byte col,byte row,char extra=' '){
    if(curLcd==NULL) return;
    // Line 1 of the one digit number
    curLcd->setCursor(col,row+0);
    curLcd->write(bn1[digit*3]);
    curLcd->write(bn1[digit*3+1]);
    curLcd->write(bn1[digit*3+2]);
    curLcd->write(extra==':' ?DOTCHAR:' ');
  
    // Line 2 of the one-digit number
    curLcd->setCursor(col,row+1);
    curLcd->write(bn2[digit*3]);
    curLcd->write(bn2[digit*3+1]);
    curLcd->write(bn2[digit*3+2]);
    curLcd->write(extra=='-'? DOTCHAR : ' ');
    // Line 3 of the one-digit number
    curLcd->setCursor(col,row+2);
    curLcd->write(bn3[digit*3]);
    curLcd->write(bn3[digit*3+1]);
    curLcd->write(bn3[digit*3+2]);
    curLcd->write(extra=='.' || extra==':' ?DOTCHAR:' ');
  }

  void printBigNumber(unsigned int num,int col=19-3,int row=1){
    while(col>=0){
      if(col==19-3 || num!=0){
        int digit=num%10;
        num/=10;
        printBigDigit(digit,col,row);
      }else{
        eraseBigDigit(col,row);
      }
      col-=4;
    }
  }

  void printSpeed(int speed,int col=19-3-4,int row=1){
      eraseBigDigit(19-3,1);
      printBigNumber(speed,19-3-4);
      setCursor(19-4,1); print(" inch");
      setCursor(19-4,2); print(" per ");
      setCursor(19-4,3); print(" sec.");
  }

  //12345 displays as 12.345
  void printMillisAsSeconds(unsigned long num,int col=19-3,int row=1){
    auto orig=num;
    int period=3;
    while(num>99999){
      num/=10;
      period--;
    }
    if(period<0){
       setCursor(0,row);
       print(num);
       return;
    }
    while(col>=0){
      if(col==19-3 || num!=0){
        int digit=num%10;
        num/=10;
        printBigDigit(digit,col,row,period==0?'.':' ');
      }else{
        eraseBigDigit(col,row);
      }
      col-=4;
      period--;
    }
    setCursor(19,3);
    print("S");
  }

  void begin(int a,int b){
    if(curLcd==NULL) return;
    #ifdef USELCD
    //initialize lcd screen
    curLcd->begin(20,4);
    curLcd->init();
    // turn on the backlight
    curLcd->backlight();

    defineLargeChars();
    #endif
  }

  

  void setCursor(int col,int row){
    if(curLcd==NULL) return;
    #ifdef USELCD
      curLcd->setCursor(col,row);
    #endif
  }
    
  // Templated member function
  template <typename T>
  void print(const T& value) {
    if(curLcd==NULL) return;
    #ifdef USELCD
     curLcd->print(value);
     scrolled=millis()+2000;
    #endif
  }

  // Templated member function
  template <typename T>
  void print(int c,int r,const T& value) {
    if(curLcd==NULL) return;
    #ifdef USELCD
     setCursor(c,r);
     curLcd->print(value);
     scrolled=millis()+2000;
    #endif
  }

  void print(char* label,unsigned long lt){
    if(curLcd==NULL) return;
    #ifdef USELCD
      curLcd->print(label);
      if(lt<10000) {
        curLcd->print(lt);
        curLcd->print("ms");
      }else if(lt>60000){
        lt/=100;
        curLcd->print(lt/10.0);
        curLcd->print("s");
      }else{
        curLcd->print(lt/1000.0);
        curLcd->print("s");
      }
    #endif
  }
  
  

};


// Array index into parts of big numbers. Numbers consist of 9 custom characters in 3 lines
// 0 1 2 3 4 5 6 7 8 9 
static const char MyLCD::bn1[]{B,2,1, 2,1,A, 2,2,1, 2,2,1, 3,A,B, B,2,2, B,2,2, 2,2,B, B,2,1, B,2,1};
static const char MyLCD::bn2[]{B,A,B, A,B,A ,3,2,2, A,6,1, 5,6,B, 5,6,7, B,6,7, A,3,2, B,6,B, 5,6,B};
static const char MyLCD::bn3[]{4,3,B, 3,B,3, B,3,3, 3,3,B, A,A,B, 3,3,B, 4,3,B, A,B,A, 4,3,B, A,A,B};


//EOF
