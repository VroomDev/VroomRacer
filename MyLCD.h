/*
   VroomRacer by Chris Busch (c) 2024
   There are no warranties express or implied with this code.
   No guarantees of being fit for purpose.
*/


#define USEHDLIB 1

#define USELCD 1

#ifdef USELCD
#include <Wire.h>
#ifdef USEHDLIB
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header    
#else
#include <LiquidCrystal_I2C.h>
#endif

#endif

//initialize the liquid crystal library
//the first parameter is  the I2C address
//the parameter is how many columns are on your screen
//the parameter is how many rows are on your screen
#ifdef USELCD
#define MAXLCDS 2
#ifdef USEHDLIB
// Change the pointer type here:
hd44780_I2Cexp* plcds[MAXLCDS] {NULL};
hd44780_I2Cexp* curLcd = NULL;
#else
LiquidCrystal_I2C* plcds[MAXLCDS] {NULL}; //pointers to LCDs   //realLcd(0x26, 20, 4);
LiquidCrystal_I2C* curLcd = NULL;
#endif
#endif

int nDevices;

//big font code is refactored/based on Hifiduino https://downloadcode.wordpress.com/2009/06/22/code-v-0-7/

#define B ((char)0xFF)             // The character for a completely filled box
#define A ((char)0x20)             // The character for blank


int scanDevices() {
  byte error, address;
  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ ) {
    Serial.println(address);
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      //hd44780_I2Cexp
#ifdef USEHDLIB
      curLcd = plcds[nDevices] = new hd44780_I2Cexp(address);
      // Initialize with begin() - returns 0 on success
      if (curLcd->begin(20, 4) == 0) {
        curLcd->backlight();
        curLcd->setCursor(0, 0);
        curLcd->print(title);
        curLcd->setCursor(5, 3);
        curLcd->print("LCD 0x");
        curLcd->print(address, HEX);
        nDevices++;
      } else {
        Serial.println("LCD init failed!");
      }
#else
      curLcd = plcds[nDevices] = new LiquidCrystal_I2C(address, 20, 4);
      curLcd->init();
      curLcd->backlight();
      curLcd->setCursor(0, 0);
      curLcd->print(title);
      curLcd->setCursor(5, 3);
      curLcd->print("LCD 0x");
      curLcd->print(address, HEX);
      nDevices++;
#endif

      delay(50);
      if (nDevices == MAXLCDS) return nDevices;
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

void setDevice(int n);

class MyLCD {



  public:



    // Array index into parts of big numbers. Numbers consist of 9 custom characters in 3 lines
    // 0 1 2 3 4 5 6 7 8 9
    static const char bn1[];//{B,2,1, 2,1,A, 2,2,1, 2,2,1, 3,A,B, B,2,2, B,2,2, 2,2,B, B,2,1, B,2,1};
    static const char bn2[];//{B,A,B, A,B,A ,3,2,2, A,6,1, 5,6,B, 5,6,7, B,6,7, A,3,2, B,6,B, 5,6,B};
    static const char bn3[];//{4,3,B, 3,B,3, B,3,3, 3,3,B, A,A,B, 3,3,B, 4,3,B, A,B,A, 4,3,B, A,A,B};

    void boot() {
      defineLargeChars();
      printBigString("VR00M!", 5, 1);
    }

    void clear() {
      if (curLcd == NULL) return;
      curLcd->clear();
    }


    void defineBarChart() {
      curFont = CustomFont::BAR;
      uint8_t cc1[8] = {     // Custom Character 1
        B01110,
        B01110,
        B01110,
        B01110,
        B01110,
        B01110,
        B01110,
        B01110
      };
      for (int i = 0; i < 8; i++) {
        curLcd->createChar(7 - i, cc1);
        cc1[i] = 0;
      }
    }


    // The routine to create the custom characters in the LCD
    void defineLargeChars() {
      curFont = CustomFont::BIG;
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

      if (curLcd == NULL) return;

      // send custom characters to the display
      curLcd->createChar(1, cc1);
      curLcd->createChar(2, cc2);
      curLcd->createChar(3, cc3);
      curLcd->createChar(4, cc4);
      curLcd->createChar(5, cc5);
      curLcd->createChar(6, cc6);
      curLcd->createChar(7, cc7);

    }






    void clearLine() {
      if (curLcd == NULL) return;
#ifdef USELCD
      ///////////////         1         2            3
      ///////////////1234567890123456789012345678901234567890
      curLcd->print("                    ");
#endif
    }

  public:

    enum class CustomFont {
      UNDEFINED,  // Defaults to 0
      BIG,  // Defaults to 1
      BAR    // Defaults to 2
    };

    static CustomFont curFont;
    int lefts;
    unsigned long scrolled;

    MyLCD(): lefts(0), scrolled(millis()) {}


    void ensureFont(CustomFont f) {
      if (f != curFont) {
        if (f == CustomFont::BIG) {
          defineLargeChars();
        } else if (f == CustomFont::BAR) {
          defineBarChart();
        }
      }
      curFont = f;
    }



    //fills the entire line
    void printRow(int r, const char* msg) {
      char buffer[40] = "";
      snprintf(buffer, sizeof(buffer), "%-20s", msg);
      buffer[20] = 0; //null term
      setCursor(0, r);
      print(buffer);
    }

    void printRowBoth(int r, const char* msg) {
      for (int i = 0; i < nDevices; i++) {
        setDevice(i);
        printRow(r, msg);
      }
    }

    void eraseBigDigit(byte col, byte row) {
      if (curLcd == NULL) return;
      curLcd->setCursor(col, row + 0);
      curLcd->print("    ");
      curLcd->setCursor(col, row + 1);
      curLcd->print("    ");
      curLcd->setCursor(col, row + 2);
      curLcd->print("    ");
    }

#define DOTCHAR 'o'

    void printBigDigit(uint8_t digit, byte col, byte row, char extra = ' ') {
      if (curLcd == NULL) return;
      ensureFont(CustomFont::BIG);
      // Line 1 of the one digit number
      curLcd->setCursor(col, row + 0);
      curLcd->write(bn1[digit * 3]);
      curLcd->write(bn1[digit * 3 + 1]);
      curLcd->write(bn1[digit * 3 + 2]);
      curLcd->write(extra == ':' ? DOTCHAR : ' ');

      // Line 2 of the one-digit number
      curLcd->setCursor(col, row + 1);
      curLcd->write(bn2[digit * 3]);
      curLcd->write(bn2[digit * 3 + 1]);
      curLcd->write(bn2[digit * 3 + 2]);
      curLcd->write(extra == '-' ? DOTCHAR : ' ');
      // Line 3 of the one-digit number
      curLcd->setCursor(col, row + 2);
      curLcd->write(bn3[digit * 3]);
      curLcd->write(bn3[digit * 3 + 1]);
      curLcd->write(bn3[digit * 3 + 2]);
      curLcd->write(extra == '.' || extra == ':' ? DOTCHAR : ' ');
    }

    void printBigString(const char* msg, int col = 0, int row = 1) {
      bool needpad = false;
      for (int i = 0; msg[i] != 0 && col < 20 && row < 4; i++) {
        char c = msg[i];
        if (c >= '0' && c <= '9') {
          if (needpad) col++;
          printBigDigit(c - '0', col, row);
          col += 3;
          needpad = true;
        } else if (c == ' ') {
          if (needpad) col++;
          eraseBigDigit(col, row);
          col += 3;
          needpad = true;
        } else {
          setCursor(col, row);
          print((c == '|' | c == ':') ? c : ' ');
          setCursor(col, row + 1);
          print((c == '|' | c == ':') ? c : ' ');
          setCursor(col, row + 2);
          print(c);
          col++;
        }
      }
    }

    void printBigNumber(unsigned int num, int col = 19 - 3, int row = 1) {
      while (col >= 0) {
        if (col == 19 - 3 || num != 0) {
          int digit = num % 10;
          num /= 10;
          printBigDigit(digit, col, row);
        } else {
          eraseBigDigit(col, row);
        }
        col -= 4;
      }
    }

    void printSpeed(int speed, int col = 19 - 3 - 4, int row = 1) {
      eraseBigDigit(19 - 3, 1);
      printBigNumber(speed, 19 - 3 - 4);
      setCursor(19 - 4, 1); print(" inch");
      setCursor(19 - 4, 2); print(" per ");
      setCursor(19 - 4, 3); print(" sec.");
    }

    void printReaction(int millis, int col = 19 - 3 - 4, int row = 1) {
      eraseBigDigit(19 - 3, 1);
      printBigNumber(millis, 19 - 3 - 4);
      setCursor(19 - 4, 1); print(" Rxn ");
      setCursor(19 - 4, 2); print(" Time");
      setCursor(19 - 4, 3); print(" (MS)");
    }


    //12345 displays as 12.345
    void printMillisAsSeconds(unsigned long num, int col = 19 - 3, int row = 1) {
      auto orig = num;
      int period = 3;
      while (num > 99999) {
        num /= 10;
        period--;
      }
      if (period < 0) {
        setCursor(0, row);
        print(num);
        return;
      }
      while (col >= 0) {
        if (col == 19 - 3 || num != 0) {
          int digit = num % 10;
          num /= 10;
          printBigDigit(digit, col, row, period == 0 ? '.' : ' ');
        } else {
          eraseBigDigit(col, row);
        }
        col -= 4;
        period--;
      }
      setCursor(19, 3);
      print("S");
    }



    void setCursor(int col, int row) {
      if (curLcd == NULL) return;
#ifdef USELCD
      curLcd->setCursor(col, row);
#endif
    }

    // Templated member function
    template <typename T>
    void print(const T& value) {
      if (curLcd == NULL) return;
#ifdef USELCD
      curLcd->print(value);
      scrolled = millis() + 2000;
#endif
    }

    // Templated member function
    template <typename T>
    void print(int c, int r, const T& value) {
      if (curLcd == NULL) return;
#ifdef USELCD
      setCursor(c, r);
      curLcd->print(value);
      scrolled = millis() + 2000;
#endif
    }

    void print(char* label, unsigned long lt) {
      if (curLcd == NULL) return;
#ifdef USELCD
      curLcd->print(label);
      if (lt < 10000) {
        curLcd->print(lt);
        curLcd->print("ms");
      } else if (lt > 60000) {
        lt /= 100;
        curLcd->print(lt / 10.0);
        curLcd->print("s");
      } else {
        curLcd->print(lt / 1000.0);
        curLcd->print("s");
      }
#endif
    }



    /**
       @param data Array of 20 integers (0-32)
       @param len  Length of the array (up to 20)
    */
    void drawBarChart(int8_t data[], int len) {
      if (curLcd == NULL) return;
      ensureFont(CustomFont::BAR);
      for (int col = 0; col < len && col < 20; col++) {
        int value = data[col];

        // Constrain value
        for (int row = 0; row < 4; row++) {
          // We process from bottom (row 3) to top (row 0)
          // Row 3 handles values 1-8
          // Row 2 handles values 9-16
          // Row 1 handles values 17-24
          // Row 0 handles values 25-32
          int displayRow = 3 - row;

          // Calculate how many units apply to this specific 8-pixel row
          int rowValue = value - (row * 8);
          curLcd->setCursor(col, displayRow);
          if (value > 33) {
            curLcd->write( displayRow == 3 ? (char)value : ' '); //overflow, so print as ASCII character
          } else if (value > 32) {
            curLcd->write((char)value); //overflow, so print as ASCII character
          } else if (value < 0) {
            curLcd->write( displayRow == 3 ? 'x' : ' '); //underflow
          } else if (value == 0) {
            curLcd->write( displayRow == 3 ? '0' : ' '); //zero
          } else if (rowValue >= 8) {
            // This row is completely full
            // Using custom char 7 (assuming 8 lines tall is index 7)
            // Note: If 'B' (0xFF) is your full block, use B instead.
            curLcd->write(7);
          }
          else if (rowValue > 0) {
            // This row is partially full (1 to 7 lines)
            // rowValue 1 -> index 0, rowValue 7 -> index 6
            curLcd->write(rowValue - 1);
          }
          else {
            // This row is empty
            curLcd->write(' ');
          }
        }
      }
    }

    void demoBarChart() {
      //      uint8_t myStats[20] = {0, 4, 8, 12, 16, 20, 24, 28, 32, 30, 25, 20, 15, 10, 5, 2, 0, 0, 0, 0};
      //      for (int i = 0; i < 16; i++) {
      //        drawBarChart(myStats, 20);
      //        for (int j = 1; j < sizeof(myStats); j++) {
      //          myStats[j - 1] = myStats[j];
      //        }
      //        delay(15);
      //      }
    }

};


// Array index into parts of big numbers. Numbers consist of 9 custom characters in 3 lines
// 0 1 2 3 4 5 6 7 8 9
const char MyLCD::bn1[] {B, 2, 1, 2, 1, A, 2, 2, 1, 2, 2, 1, 3, A, B, B, 2, 2, B, 2, 2, 2, 2, B, B, 2, 1, B, 2, 1};
const char MyLCD::bn2[] {B, A, B, A, B, A , 3, 2, 2, A, 6, 1, 5, 6, B, 5, 6, 7, B, 6, 7, A, 3, 2, B, 6, B, 5, 6, B};
const char MyLCD::bn3[] {4, 3, B, 3, B, 3, B, 3, 3, 3, 3, B, A, A, B, 3, 3, B, 4, 3, B, A, B, A, 4, 3, B, A, A, B};


MyLCD::CustomFont MyLCD::curFont = MyLCD::CustomFont::UNDEFINED;

void setDevice(int n) {
  if (n >= nDevices) n = 0; //use 0 if you can if no other
  if (n < nDevices) curLcd = plcds[n];
  MyLCD::curFont = MyLCD::CustomFont::UNDEFINED;
}


//EOF
