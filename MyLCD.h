

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
LiquidCrystal_I2C realLcd(0x27, 20, 4);
#endif


//big font code is refactored/based on Hifiduino https://downloadcode.wordpress.com/2009/06/22/code-v-0-7/

#define B 0xFF             // The character for a completely filled box
#define A 0x20             // The character for blank



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
  
    // send custom characters to the display
    realLcd.load_custom_character(1,cc1);
    realLcd.load_custom_character(2,cc2);
    realLcd.load_custom_character(3,cc3);
    realLcd.load_custom_character(4,cc4);
    realLcd.load_custom_character(5,cc5);
    realLcd.load_custom_character(6,cc6);
    realLcd.load_custom_character(7,cc7);
  
  }
  



  
  
  void clearLine(){
    #ifdef USELCD
    ///////////////         1         2            3
    ///////////////1234567890123456789012345678901234567890
    realLcd.print("                    ");
    #endif
  }
  
  public:

  int lefts;
  unsigned long scrolled;

  MyLCD():lefts(0),scrolled(millis()){}


  // Routines for LCD Adjustment
  
  // For LCD backlight adjustment
  void backLight(uint8_t bright){
    Wire.beginTransmission(0x4C);
    Wire.write(0xFE);
    Wire.write(0x03);
    Wire.write(bright);
    Wire.endTransmission();
    delay(25);
  }
  
  // For LCD contrast adjustment
  void contrast(uint8_t cont) {
    Wire.beginTransmission(0x4C);
    Wire.write(0xFE);
    Wire.write(0x04);
    Wire.write(cont);
    Wire.endTransmission();
    delay(25);
  }


  void eraseBigDigit(byte col, byte row){
    realLcd.setCursor(col,row+0);
    realLcd.print("    ");
    realLcd.setCursor(col,row+1);
    realLcd.print("    ");
    realLcd.setCursor(col,row+2);
    realLcd.print("    ");
  }

  #define DOTCHAR 'o'

  void printBigDigit(uint8_t digit,byte col,byte row,char extra=' '){
    // Line 1 of the one digit number
    realLcd.setCursor(col,row+0);
    realLcd.write(bn1[digit*3]);
    realLcd.write(bn1[digit*3+1]);
    realLcd.write(bn1[digit*3+2]);
    realLcd.write(extra==':' ?DOTCHAR:' ');
  
    // Line 2 of the one-digit number
    realLcd.setCursor(col,row+1);
    realLcd.write(bn2[digit*3]);
    realLcd.write(bn2[digit*3+1]);
    realLcd.write(bn2[digit*3+2]);
    realLcd.write(extra=='-'? DOTCHAR : ' ');
    // Line 3 of the one-digit number
    realLcd.setCursor(col,row+2);
    realLcd.write(bn3[digit*3]);
    realLcd.write(bn3[digit*3+1]);
    realLcd.write(bn3[digit*3+2]);
    realLcd.write(extra=='.' || extra==':' ?DOTCHAR:' ');
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
    setCursor(19,2);
    print("S");
  }

  void begin(int a,int b){
    #ifdef USELCD
    //initialize lcd screen
    realLcd.begin(20,4);
    realLcd.init();
    // turn on the backlight
    realLcd.backlight();

    defineLargeChars();
    #endif
  }

  

  void setCursor(int col,int row){
    #ifdef USELCD
      realLcd.setCursor(col,row);
    #endif
  }
    
  // Templated member function
  template <typename T>
  void print(const T& value) {
    #ifdef USELCD
     realLcd.print(value);
     scrolled=millis()+2000;
    #endif
  }

  void print(char* label,unsigned long lt){
    #ifdef USELCD
      realLcd.print(label);
      if(lt<10000) {
        realLcd.print(lt);
        realLcd.print("ms");
      }else if(lt>60000){
        lt/=100;
        realLcd.print(lt/10.0);
        realLcd.print("s");
      }else{
        realLcd.print(lt/1000.0);
        realLcd.print("s");
      }
    #endif
  }
  
  

};


// Array index into parts of big numbers. Numbers consist of 9 custom characters in 3 lines
// 0 1 2 3 4 5 6 7 8 9 
static const char MyLCD::bn1[]{B,2,1, 2,1,A, 2,2,1, 2,2,1, 3,A,B, B,2,2, B,2,2, 2,2,B, B,2,1, B,2,1};
static const char MyLCD::bn2[]{B,A,B, A,B,A ,3,2,2, A,6,1, 5,6,B, 5,6,7, B,6,7, A,3,2, B,6,B, 5,6,B};
static const char MyLCD::bn3[]{4,3,B, 3,B,3, B,3,3, 3,3,B, A,A,B, 3,3,B, 4,3,B, A,B,A, 4,3,B, A,A,B};
