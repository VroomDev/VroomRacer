////////////////////////Lane.h by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
#include <Arduino.h> 
const int minLapDuration = 500; // Debounce delay in milliseconds
#define PAGECOUNT 5
unsigned int tock=0;

class Lane {
  static const bool serialOn=true;
  public:

    int laneNum = 0;
    unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
    int speed=0;    
    int speedCount=0;
    bool crossedStart=false;
    int lapCounter=0;
    unsigned long lapDuration=0,bestLapDur=0,worstLapDur=0,avgLapDur=0,totalSpeed=0;
    unsigned long topSpeed=0,lowSpeed=0;
    unsigned long long totalDuration=0;
       
    void setup(int pin){
      laneNum=pin;
      lastLapTime=millis();
    }

   int setSpeed(int s){
    speed=s;    
    if(speedCount<1024){
      totalSpeed+=s;
      speedCount++;
    }
    if(s>topSpeed){
      topSpeed=s;
    }else if(s < lowSpeed || lowSpeed==0){
      lowSpeed=s;      
    }
    return s;
  }

  int avgSpeed(){
      if(speedCount==0) return 0;
      return totalSpeed/speedCount;
  }


  Detection prior;
  
  bool detect(Detection d){
    //LAP DETECTED
    lastLapTime = d.timestamp;
    if(prior.isEmpty()){
      prior=d;
      lapDuration=0; //race started crossed finish for first time   
      crossedStart=true; 
      return false;      
    }else{
      lapCounter++; 
      lapDuration = d.timestamp-prior.timestamp;
      totalDuration+=lapDuration;      
      if(lapDuration<bestLapDur || bestLapDur==0) bestLapDur=lapDuration; //always set best first
      else if(lapDuration>worstLapDur || worstLapDur==0) worstLapDur=lapDuration;
      avgLapDur = totalDuration/lapCounter;
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



  void display(byte page,char flag){
    if(NUMLANES==2) display2(page,flag);
    else display4(page,flag);
  }

  const char* flagToString(RaceFlag flag){
    switch(flag){
      case FORMATION: return "RDY";
      case REDFLAG: return "RED";
      case YELLOWFLAG: return "YEL";
      case GREENFLAG: return "GRN";
      case CHECKERS: return "OVR";
      default: return "  ";
    }
  }

  void display2(byte page,RaceFlag flag){    
    lcd.setCursor(0,laneNum*2);//col,row    
    char floatBuffer1[10]; // Buffer to hold the formatted float     
    char floatBuffer2[10]; // Buffer to hold the formatted float     
    char buffer[40];
    
    ////////////
    /// first line
    ///////////
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';     
    if(lapCounter==0){
      if(crossedStart){
        ////////////////01234567890123456789
        sprintf(buffer,"%c%d %3s Started! ",ch,laneNum,flagToString(flag));
      }else{
        sprintf(buffer,"%c%d %3s Go!        ",ch,laneNum,flagToString(flag));
      }
    }else{    
      mydtostrf((lapDuration / 1000.0), 5, floatBuffer1); // Convert float to string
      mydtostrf(avgLapDur/1000.0, 4, floatBuffer2); // Convert float to string    
      sprintf(buffer,"%c%d %s Last:%5ss",ch,laneNum,flagToString(flag),floatBuffer1);
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);    
    if(serialOn) {
      Serial.print(buffer);
      Serial.print("\n");     
    }
    //////////////
    ////// second line
    //////////////
    lcd.setCursor(0,laneNum*2+1);//col,row    
    //        012345678901234567890
    //   all: C0 12.400s A:12.40s
    //page 0: L00 Trap0000i/s Gr
    //page 1: C0 L10 Last 12.400s F
    //page 2: C0 L10 Avg  12.400s F
    //page 3: C0 L10 Best 12.400s F
    //page 4: C0 L10 Slow 112.40s F
        
    switch(page){
      case 0:
        sprintf(buffer,"%c%d Lap%-2d Spd%4din/s",ch,laneNum,(int)lapCounter,(int)speed);
        break;
//      case 0:
//        mydtostrf((lapDuration / 1000.0), 5, floatBuffer1); // Convert float to string
//        sprintf(buffer,"%c%d Lap%-2d Last %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1);
//        break;
      case 1:
        mydtostrf(avgLapDur/1000.0, 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d Lap%-2d Avg  %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1);
        break;
      case 2:
        mydtostrf((bestLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d Lap%-2d Best %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1);
        break;
      case 3:
        sprintf(buffer,"%c%d Lap%-2d Top%4din/s",ch,laneNum,(int)lapCounter,(int)topSpeed);
        break;
      default: //3
        mydtostrf((worstLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d Lap%-2d Slow %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1);
        break;
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);    
    if(serialOn) {
      Serial.print(buffer);
      Serial.print("\n");
    }     
  }

  void display4(byte page,char flag){    
     lcd.setCursor(0,laneNum);//col,row    
    //        012345678901234567890
    //page 0: C0 L10 0000inch/sec F
    //page 1: C0 L10 Last 12.400s F
    //page 2: C0 L10 Avg  12.400s F
    //page 3: C0 L10 Best 12.400s F
    //page 4: C0 L10 Slow 112.40s F
    char buffer[40];
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';     
    char floatBuffer1[10]; // Buffer to hold the formatted float     
    switch(page){
      case 1:
        sprintf(buffer,"%c%d L%-2d%4dinch/sec %c",ch,laneNum,(int)lapCounter,(int)speed,flag);
        break;
      case 0:
        mydtostrf((lapDuration / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Last %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1,flag);
        break;
      case 2:
        mydtostrf(avgLapDur/1000.0, 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Avg  %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1,flag);
        break;
      case 3:
        mydtostrf((bestLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Best %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1,flag);
        break;
      default:
        mydtostrf((worstLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Slow %5ss %c",ch,laneNum,(int)lapCounter,floatBuffer1,flag);
        break;
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);    
    if(serialOn) Serial.print(buffer);     
  }
 



  
};
