
//************************************************************************
//*      A set of custom made large numbers for a 16x2 LCD using the
//*      LiquidCrystal librabry. Works with displays compatible with the
//*      Hitachi HD44780 driver.
//*      
//*      orginal developed  by Michael Pilcher  2/9/2010
//*      there are 8 entries, 8 bytes per entry
//*      these are the building blocks to make the numbers
//*      
//*      Updated by Chris B 2025 with font fixes and simpler code.
//************************************************************************


//*      The LL, LT, UB, ect... are rust abreviations to help me designate which segment was which when referencing the large '0'.
//*      LT= left top
//*      UB= upper bar
//*      RT= right top
//*      LL= lower left
//*      LB= lower bar
//*      LR= lower right
//*      UMB= upper middle bars(upper middle section of the '8')
//*      LMB= lower middle bars(lower middle section of the '8')

const uint8_t      gBigFontShapeTable[] PROGMEM      =      {
//*      LT[8] =  0
  B00111,
  B01111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
//*      UB[8] =1
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
//*      RT[8] =2
  B11100,
  B11110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
//*      LL[8] =3
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B01111,
  B00111,
//*      LB[8] =4
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111,
//*      LR[8] =5
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11110,
  B11100,
//*      UMB[8] =6
  B11111,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
//*      LMB[8] =7
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B11111,
  B11111


};

#define BOX 0xFF             // The character for a completely filled box
//************************************************************************
//*      6 numbers for each character
//*      9 means BLANK
const uint8_t      gBigFontAsciiTable[] PROGMEM      =      {

      32,      32,      32,      32,      32,      32,            //      0x20      space
      32,      0,      32,      32,      4,      32,            //      0x21      !
      32,      32,      32,      32,      32,      32,            //      0x22      
      32,      32,      32,      32,      32,      32,            //      0x23      
      32,      32,      32,      32,      32,      32,            //      0x24      
      32,      32,      32,      32,      32,      32,            //      0x25      
      32,      32,      32,      32,      32,      32,            //      0x26      
      32,      32,      32,      32,      32,      32,            //      0x27      
      32,      32,      32,      32,      32,      32,            //      0x28      
      32,      32,      32,      32,      32,      32,            //      0x29      
      32,      32,      32,      32,      32,      32,            //      0x2A      
      32,      32,      32,      32,      32,      32,            //      0x2B      
      32,      32,      32,      32,      32,      32,            //      0x2C      
      32,      32,      32,      32,      32,      32,            //      0x2D
      32,      32,      32,      'o'/*4*/,      32,      32,            //      0x2E      . (period)
      32,      32,      32,      32,      32,      32,            //      0x2F      


      0,      1,      2,      3,      4,      5,            //      0x30      0
      1,      2,      32,      4,      BOX,      4,            //      0x31      1
      6,      6,      2,      3,      7,      7,            //      0x32      2
      6,      6,      2,      7,      7,      5,            //      0x33      3
      3,      4,      2,      32,      32,      5,            //      0x34      4
      255,6,      6,      7,      7,      5,            //      0x35      5 
//      0,      6,      6,      7,      7,      5,            //      0x35      5
      0,      6,      6,      3,      7,      5,            //      0x36      6
      1,      1,      2,      32,      0,      32,            //      0x37      7
      0,      6,      2,      3,      7,      5,            //      0x38      8
      0,      6,      2,      32,      32,      5,            //      0x39      9
      32,      32,      32,      32,      32,      32,            //      0x3A      
      32,      32,      32,      32,      32,      32,            //      0x3B
      32,      32,      32,      32,      32,      32,            //      0x3C
      32,      32,      32,      32,      32,      32,            //      0x3D
      32,      32,      32,      32,      32,      32,            //      0x3E
      1,      6,      2,      254,7,      32,            //      0x3F      ?

      32,      32,      32,      32,      32,      32,            //      0x40      @
      0,      6,      2,      255,254,255,      //      0x41      A
      255,6,      5,      255,7,      2,            //      0x42      B
      0,      1,      1,      3,      4,      4,            //      0x43      C
      255,1,      2,      255,4,      5,            //      0x44      D
      255,6,      6,      255,7,      7,            //      0x45      E
      255,6,      6,      255,32,      32,            //      0x46      F
      
      0,      1,      1,      3,      4,      2,            //      0x47      G
      255,4,      255,255,254,255,      //      0x48      H
      1,      255,1,      4,      255,4,            //      0x49      I
      32,      32,      255,4,      4,      5,            //      0x4A      J
      255,4,      5,      255,254,2,            //      0x4B      K
      255,32,      32,      255,4,      4,            //      0x4C      L
      32,      32,      32,      32,      32,      32,            //      0x4D      M      place holder
      32,      32,      32,      32,      32,      32,            //      0x4E      N      place holder
      0,      1,      2,      3,      4,      5,            //      0x4F      O      (same as zero)

      0,      6,      2,      3,      32,      32,            //      0x50      P
      32,      32,      32,      32,      32,      32,            //      0x51      Q
      0,      6,      5,      3,      32,      2,            //      0x52      R
      0,      6,      6,      7,      7,      5,            //      0x35      S      (same as 5)
      1,      2,      1,      32,      5,      32,            //      0x54      T
      2,      32,      2,      3,      4,      5,            //      0x55      U
      3,      32,        5,         32,      3,        32,            //      0x56      V       
      32,      32,      32,      32,      32,      32,            //      0x57      W      place holder
      3,      4,      5,      0,      32,      2,            //      0x58      X
      3,      4,      5,      32,      5,      32,            //      0x59      Y
      1,      6,      5,      0,      7,      4,            //      0x5A      Z
      0

};

//*      we have a seperate table for the wide routines
const uint8_t      gBigFontAsciiTableWide[] PROGMEM      =      {
//*      this table is 10 bytes, 2 rows of 5
//      ---top------------|
      0,      1,      3,      1,      2,      3,      32,      32,      32,      5,            //      0x4D      M            5-wide
      0,      3,      32,      2,      32,      3,      32,      2,      5,      32,            //      0x4E      N            4-wide
      0,      1,      2,      32,      32,      3,      4,      3,      4,      32,            //      0x51      Q            4-wide
      3,      32,      32,      5,      32,      32,      3,      5,      32,      32,            //      0x56      V            4-wide
      0,      32,      32,      32,      2,      3,      4,      0,      4,      5,            //      0x57      W            5-wide
      0
};


//************************************************************************
static void BigNumber_SendCustomChars(void) {
  uint8_t customCharDef[10];
  uint8_t ii;
  int jj;

  for (ii = 0; ii < 8; ii++) {
    for (jj = 0; jj < 8; jj++) {
      customCharDef[jj] = pgm_read_byte_near(gBigFontShapeTable + (ii * 8) + jj);
    }
    gLCD.createChar(ii, customCharDef);
  }
}


//************************************************************************
//* returns the width of the character
static int drawBigChar(int xLocation, int yLocation, char theChar) {
  int offset;
  int ii;
  char theByte;
  boolean specialCase;
  int specialIndex;
  int charWidth;
//
//  if (theChar == 0x20) {
//    return 2;
//  } else 
  if (theChar < 0x20) {
    return 0;
  }

  if (theChar >= 'A') {
    theChar = theChar & 0x5F;  //* force to upper case
  }
  specialCase = true;
  switch (theChar) {
    case 'M':
      charWidth = 5;
      specialIndex = 0;
      break;
    case 'N':
      charWidth = 4;
      specialIndex = 1;
      break;
    case 'Q':
      charWidth = 4;
      specialIndex = 2;
      break;
//    case 'V':   //looks fine only 3 wide
//      charWidth = 4;
//      specialIndex = 3;
//      break;
    case 'W':
      charWidth = 5;
      specialIndex = 4;
      break;
    default:
      charWidth = 3;
      switch(theChar){
        case '.': charWidth=1;
        break;
      }
      specialCase = false;
      offset = 6 * (theChar - 0x20);
      gLCD.setCursor(xLocation, yLocation);
      for (ii = 0; ii < charWidth; ii++) {
        theByte = pgm_read_byte_near(gBigFontAsciiTable + offset + ii);
        gLCD.write(theByte);
      }
      if(theChar!='.') gLCD.write(' ');
      gLCD.setCursor(xLocation, yLocation + 1);
      offset += 3;
      for (ii = 0; ii < charWidth; ii++) {
        theByte = pgm_read_byte_near(gBigFontAsciiTable + offset + ii);
        gLCD.write(theByte);
      }
      if(theChar!='.') gLCD.write(' ');
      break;
  }
  if (specialCase) {
    offset = 10 * specialIndex;
    gLCD.setCursor(xLocation, yLocation);
    for (ii = 0; ii < charWidth; ii++) {
      theByte = pgm_read_byte_near(gBigFontAsciiTableWide + offset + ii);
      gLCD.write(theByte);
    }
    gLCD.write(' ');
    gLCD.setCursor(xLocation, yLocation + 1);
    offset += 5;
    for (ii = 0; ii < charWidth; ii++) {
      theByte = pgm_read_byte_near(gBigFontAsciiTableWide + offset + ii);
      gLCD.write(theByte);
    }
    gLCD.write(' ');
  }
  
  return (charWidth + (theChar=='.'?0:1));
}

static int drawString(int col,int row,char* msg){
  for(int i=0;msg[i]!=0 && i<20;i++){
    if(i>0 && msg[i]=='.') col--;
    col+=drawBigChar(col,row,msg[i]);
  }
}

int printWrap(int col,int row,char* msg,int wrapcol=20,int wraprow=4){
  int c=col;
  lcd.setCursor(c,row);
  for(int i=0;msg[i]!=0;i++){
    lcd.print(msg[i]);
    c++;
    if(c>=wrapcol){
      c=col;
      row++;
      if(row>=wraprow) row=0;
      lcd.setCursor(c,row);
    }
  }  
}
