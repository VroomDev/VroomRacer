////////////////////////Lane.h by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
#include <Arduino.h> 
//const int minLapDuration = 2500;
#define PAGECOUNT 8
unsigned int tock=0;

class Lane {
  public:

    int laneNum = 0;
    unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
    int speed=0;    
    int speedCount=0;
    bool crossedStart=false;
    int lapCounter=0;
    static unsigned long allBestLapDur;
    
    unsigned long lapDuration=0,bestLapDur=0,worstLapDur=0,avgLapDur=0,totalSpeed=0;
    unsigned long topSpeed=0,lowSpeed=0,initialTime=0;
    unsigned long long totalDuration=0;
    char why[40]=""; //this holds the reason why a lap was not counted.
       
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

  /** return false if a bad crossing, returns true with a good start/finish crossing
  */
  bool detect(Detection d){
    //LAP DETECTED
    lastLapTime = d.timestamp;
    if(prior.isEmpty()){
      initialTime=d.timestamp;
      prior=d;
      lapDuration=0; //race started crossed finish for first time   
      crossedStart=true; 
      return true;      
    }else{      
      lapDuration = d.timestamp-prior.timestamp;
      if(lapDuration<minLapDuration){
        sprintf(why,"%d Hop",lapDuration);
        return false;
      }else if(allBestLapDur!=0 && lapDuration<allBestLapDur*6/10){
        //jumped car, since better than anyone by too large of a margin
        sprintf(why,"%d Jump",lapDuration);
        return false;
      }
      if(lapDuration<allBestLapDur || allBestLapDur==0){
         allBestLapDur=lapDuration; //used for jumped lap detection
      }
      if(lapDuration<bestLapDur || bestLapDur==0) {
        bestLapDur=lapDuration; //always set best first
      }else if(lapDuration>worstLapDur || worstLapDur==0){
        worstLapDur=lapDuration; //need to have 2 laps to have worst lap.
      }
      lapCounter++; 
      totalDuration+=lapDuration;      
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
        if(serialOn) Serial.print(buffer); 
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


  void banner(bool lapCounted,char* msg){
    setDevice(laneNum);
    lcd.setCursor(0,0);//col,row    
    char floatBuffer1[10]; // Buffer to hold the formatted float     
    char floatBuffer2[10]; // Buffer to hold the formatted float     
    char buffer[40];
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';         
                    // 1234567890123467890
    if(msg[0]==0){   //C0 Lap00 Speed12300
      sprintf(buffer,"%c%d Lap%-2d Speed%5d   "
                   ,ch,laneNum+1,(int)lapCounter,(int)speed);
    }else{           //01234567890123456789
                     //C0 Lap00 12345678901
      sprintf(buffer,"%c%d Lap%-2d %s         "
                   ,ch,laneNum+1,(int)lapCounter,msg);
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);
    if(lapCounted && (lapCounter>0) ){    
      lcd.printMillisAsSeconds(lapDuration);
    }else{
      lcd.printSpeed(speed);
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
      if(crossedStart && raceStart>0  && initialTime>0){
        ////////////////01234567890123456789
        ////////////////C1 Reaction 99999ms
        sprintf(buffer,"%c%d Reaction %ldms    ",ch,laneNum+1,((signed long)initialTime-(signed long)raceStart));
      }else{
        sprintf(buffer,"%c%d Go!              ",ch,laneNum+1);
      }
    }else{    
      mydtostrf((lapDuration / 1000.0), 5, floatBuffer1); // Convert float to string
      //mydtostrf(avgLapDur/1000.0, 4, floatBuffer2); // Convert float to string    
                    //01234567890123456789
                    //C0 Lap00 00000s Time 
      sprintf(buffer,"%c%d Lap%-2d %5ss Time            ",ch,laneNum+1,lapCounter,floatBuffer1);
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);    
    if(serialOn) {
      Serial.print(buffer);
      Serial.print("\n");     
    }
                                                //012345678901234567890
    if(raceFlag==REDFLAG)         sprintf(buffer,"Red: no laps counted.");
    else if(raceFlag==YELLOWFLAG) sprintf(buffer,"Yellow: must go slow.");
    else if(raceFlag==GREENFLAG)  sprintf(buffer,"Green: go fast!      ");
    else if(lapCounter==raceLength) sprintf(buffer,"Completed race.      ");
    else if(winner==laneNum)      sprintf(buffer,"This car won.        ");
    else                          sprintf(buffer,"                     ");
    //////////////
    ////// second line
    //////////////
    lcd.setCursor(0,laneNum*2+1);//col,row    
            //01234567890123456789
            //C0   0000 Trap Speed
            //C0  00000s Avg  Time
            //C0  00000s Best Time
            //C0   0000 Top  Speed
            //C0   0000 Avg  Speed
            //C0  00000s Slow Time   
    switch(page){
      case 0:
        if(speed!=0)        sprintf(buffer,"%c%d%7d Trap Speed  ",ch,laneNum+1,(int)speed);
        break;
      case 1:
        if(avgLapDur!=0) {
          mydtostrf(avgLapDur/1000.0, 5, floatBuffer1); // Convert float to string
          sprintf(buffer,"%c%d%7ss Avg  Time ",ch,laneNum+1,floatBuffer1);
        }
        break;
      case 2:
        if(bestLapDur!=0) {
          mydtostrf((bestLapDur / 1000.0), 5, floatBuffer1); // Convert float  to string
          sprintf(buffer,"%c%d%7ss Best Time   ",ch,laneNum+1,floatBuffer1);
        }
        break;
      case 3:         //01234567890123456789 
                      //C1 Lap00 Top000speed
        if(topSpeed!=0) {
          sprintf(buffer,"%c%d%7d Top  Speed ",ch,laneNum+1,(int)topSpeed);
        }
        break;
      case 4:
        if(avgSpeed()!=0) {
          sprintf(buffer,"%c%d%7d Avg  Speed ",ch,laneNum+1,(int)avgSpeed());
        }
        break;
      case 5:
        if(worstLapDur!=0){
           mydtostrf((worstLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
           sprintf(buffer,"%c%d%7ss Slow Time    ",ch,laneNum+1,floatBuffer1);
        }
        break;
      case 6:
        if(initialTime>0 && raceStart>0){
          sprintf(buffer,"%c%d%6ldms Reaction     ",ch,laneNum+1,((signed long)initialTime-(signed long)raceStart));
        }
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
        sprintf(buffer,"%c%d L%-2d%4dinch/sec %c",ch,laneNum+1,(int)lapCounter,(int)speed,flag);
        break;
      case 0:
        mydtostrf((lapDuration / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Last %5ss %c",ch,laneNum+1,(int)lapCounter,floatBuffer1,flag);
        break;
      case 2:
        mydtostrf(avgLapDur/1000.0, 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Avg  %5ss %c",ch,laneNum+1,(int)lapCounter,floatBuffer1,flag);
        break;
      case 3:
        mydtostrf((bestLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Best %5ss %c",ch,laneNum+1,(int)lapCounter,floatBuffer1,flag);
        break;
      default:
        mydtostrf((worstLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"%c%d L%-2d Slow %5ss %c",ch,laneNum+1,(int)lapCounter,floatBuffer1,flag);
        break;
    }
    buffer[20]=0; //null terminate
    lcd.print(buffer);    
    if(serialOn) Serial.print(buffer);     
  }
 
  
};

unsigned long Lane::allBestLapDur=0;
