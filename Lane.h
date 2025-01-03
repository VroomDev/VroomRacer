////////////////////////Lane.h by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
const int minLapDuration = 500; // Debounce delay in milliseconds

unsigned int tock=0;

class Lane {
  static const bool serialOn=true;
  public:
    int laneNum = 0;
    unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
 
    int lapCounter=0;
    unsigned long lapDuration=0,bestLapDur=0,worstLapDur=0;
       
    void setup(int pin){
      laneNum=pin;
      lastLapTime=millis();
    }

  Detection prior;
  
  bool detect(Detection d){
    //LAP DETECTED
    lastLapTime = d.timestamp;
    if(prior.isEmpty()){
      prior=d;
      lapDuration=0; //race started crossed finish for first time    
      return false;      
    }else{
      lapDuration = d.timestamp-prior.timestamp;
      if(lapDuration>worstLapDur || worstLapDur==0) worstLapDur=lapDuration;
      if(lapDuration<bestLapDur || bestLapDur==0) bestLapDur=lapDuration;
      lapCounter++;
      //lap counter is now the number of completed laps
      if(lapCounter==raceLength){
          if(won){
            //didn't win :( 
          }else{
            winner=laneNum;
            won=true;
          }
      }
      
      if(serialOn){        
        char buffer[200];
        sprintf(buffer,"C%d lapCounter:%d lapDuration:%lu \n",
          laneNum,lapCounter,lapDuration);
        if(serialOn) Serial.print(buffer); //DAYLIGHT
      }
      prior=d;
      return true;
    }
    
  }

  

  void display(){
    char buffer[100];
    lcd.setCursor(0,laneNum);//col,row
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';    
    lcd.print(ch);
    lcd.print(laneNum);
    lcd.print(" L");    lcd.print(lapCounter);
    lcd.print(" ",lapDuration);
    lcd.print(" A:",lapCounter>0 ? ((millis()-raceStart)/lapCounter):0);
    lcd.print(" B:",bestLapDur);
    lcd.print(" W:",worstLapDur);
    lcd.print(buffer);
  }
 



  
};
