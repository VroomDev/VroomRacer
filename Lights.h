

class Lights {
  public:

  byte redPin,yellowPin,greenPin;

  void setup(byte r,byte y,byte g){
    redPin=r;
    yellowPin=y;
    greenPin=g;
  }

  void waveFlag(RaceFlag which){
    switch(which){
      case FORMATION:
      all(false);
      break;
      case SET:
      red(true);
      yellow(true);
      green(true);
      break;
      case RED:      
      red(true);
      yellow(false);
      green(false);
      break;
      case YELLOW:
      red(false);
      yellow(true);
      green(false);
      break;
      case GREEN:
      red(false);
      yellow(false);
      green(true);
      break;
      case CHECKERS:
      red(true);
      yellow(true);
      green(false);
      break;
    }
  }

  void all(bool on){
    red(on);
    yellow(on);
    green(on);
  }

  void red(bool on){
    digitalWrite(redPin,on ? HIGH :LOW);
  }
  void yellow(bool on){
    digitalWrite(yellowPin,on ? HIGH :LOW);
  }
  void green(bool on){
    digitalWrite(greenPin,on ? HIGH :LOW);
  }
  

  
};
