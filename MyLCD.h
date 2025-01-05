

#define USELCD 1

#ifdef USELCD
#include <Wire.h>



/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
//#define SCROLLDELAY 700
//#include <LiquidCrystal.h>
//LiquidCrystal realLcd(7, 8, 9, 10, 11, 12);

#include <LiquidCrystal_I2C.h>
#endif

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the parameter is how many columns are on your screen
//the parameter is how many rows are on your screen
#ifdef USELCD
LiquidCrystal_I2C realLcd(0x27, 20, 4);
#endif

class MyLCD {
  private:
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

  void begin(int a,int b){
    #ifdef USELCD
    //initialize lcd screen
    realLcd.begin(20,4);
    realLcd.init();
    // turn on the backlight
    realLcd.backlight();
    #endif
  }

  

  void setCursor(int col,int row){
    #ifdef USELCD
      realLcd.setCursor(col,row);
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
