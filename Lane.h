////////////////////////Lane.h by Chris Busch
/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
#include <Arduino.h> 
//const int minLapDuration = 2500;
#define PAGECOUNT 2
unsigned int tock=0;

#define MAXFUEL 400

class Lane {
  public:

    int fuel=MAXFUEL;
    int laneNum = 0;
    unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
    int speed=0;    
    int speedCount=0;
    int fouls=0;
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
      //subtract "pit" time
      unsigned int pitTime=d.count/sensors[d.port].ticksPerMs;
      if(fuelOn){
        fuel-=speed;
      }
      lapDuration = d.timestamp-prior.timestamp - pitTime;
      if(fuel<0){
        fuel=0;
        sprintf(why,"Empty");
        return false;
      }
      if(lapDuration<minLapDuration){
        sprintf(why,"%d Hop",lapDuration);
        return false;
      }else if(allBestLapDur!=0 && lapDuration<allBestLapDur*6/10){
        //jumped car, since better than anyone by too large of a margin
        sprintf(why,"%d Jump",lapDuration);
        return false;
      }
      // if count is maxed out then not elig for best lap
      if(d.count!=MAXCOUNT && (lapDuration<allBestLapDur || allBestLapDur==0)){
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

  void gasBanner(){
    setDevice(laneNum);
    lcd.setCursor(0,0);//col,row    
    char buffer[40];
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';         
                    // 1234567890123467890
                     //C0 Lap00 In Pit G99
    sprintf(buffer,"%c%d Lap%-2d In Pit G%2d%%  "
                   ,ch,laneNum+1,(int)lapCounter,(int)((long)fuel*99/MAXFUEL));
    buffer[20]=0; //null terminate
    lcd.print(buffer);
  }

  void banner(bool lapCounted,char* msg){
    if(!lapCounted) fouls++;
    setDevice(laneNum);
    lcd.setCursor(0,0);//col,row    
    char floatBuffer1[10]; // Buffer to hold the formatted float     
    char floatBuffer2[10]; // Buffer to hold the formatted float     
    char buffer[40];
    char ch=!won?'C' : winner==laneNum ? 'W' : 'L';         
                    // 12345678901234567890
    if(msg[0]==0){   //C0 Lap00 Spd123 G23%
      sprintf(buffer,"%c%d Lap%-2d Spd%3d G%2d%%  "
                   ,ch,laneNum+1,(int)lapCounter,(int)speed,(int)((long)fuel*99/MAXFUEL));
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
    lcd.setCursor(0,0); //laneNum*2);//col,row    
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
    lcd.printRow(0,buffer);    
    if(serialOn) {
      Serial.print(buffer);
      Serial.print("\n");     
    }

    if( page ==0 ) {
      //01234567890123456789
      //Lap Time  Avg 00000s//
      //Best 00000s Gas 00% //
      //Slow 00000s Fouls 00//
      buffer[0]=0;
      if(avgLapDur!=0) {
        mydtostrf(avgLapDur/1000.0, 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"Lap Time  Avg %6ss",floatBuffer1);
      }
      lcd.printRow(1,buffer);
      buffer[0]=0;
      floatBuffer1[0]=0;
      if(bestLapDur!=0) {
        mydtostrf((bestLapDur / 1000.0), 5, floatBuffer1); // Convert float  to string
      } 
      ///////////////
      sprintf(buffer,"%4s %5s%c Gas %2d%%",bestLapDur==0?"":"Best",floatBuffer1,bestLapDur==0?' ':'s',(int)((long)fuel*99/MAXFUEL));
      
      lcd.printRow(2,buffer);
      buffer[0]=0;
      if(worstLapDur!=0){
        mydtostrf((worstLapDur / 1000.0), 5, floatBuffer1); // Convert float to string
        sprintf(buffer,"Slow %5ss Fouls%3d%c%d",floatBuffer1,fouls);
      }
      lcd.printRow(3,buffer);
    }else{
      //01234567890123456789
      //Trap Speed 0000 in/s//
      // Top 0000 Avg 0000  //  
      //  Reaction 00000s   //
      buffer[0]=0;
      if(topSpeed!=0) {
        sprintf(buffer,"Trap Speed %d in/s",(int)speed);
      }
      lcd.printRow(1,buffer);
      buffer[0]=0;
      if(avgSpeed()!=0) {
          sprintf(buffer,"Top %4d  Avg %4d",(int)topSpeed,(int)avgSpeed());
      }
      lcd.printRow(2,buffer);
      buffer[0]=0;
      if(initialTime>0 && raceStart>0){
        sprintf(buffer,"Reaction %6ldms",((signed long)initialTime-(signed long)raceStart));
      }
      lcd.printRow(3,buffer);
    }
    
                                                             //012345678901234567890
      //    if(raceFlag==REDFLAG)         sprintf(buffer,"Red: no laps counted.");
      //    else if(raceFlag==YELLOWFLAG) sprintf(buffer,"Yellow: must go slow.");
      //    else if(raceFlag==GREENFLAG)  sprintf(buffer,"Green: go fast!      ");
      //    else if(lapCounter==raceLength) sprintf(buffer,"Completed race.      ");
      //    else if(winner==laneNum)      sprintf(buffer,"This car won.        ");
      //    else                          sprintf(buffer,"                     ");
      //        
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

//EOF
