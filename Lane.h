

class Lane {
  public:
    int lightPin = 0;
    volatile int threshold = 500; // Threshold value for detection
    volatile unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
  
    volatile int reading=0;
    volatile int lapCounter=-1;
    int reportedLap=-1;
    volatile unsigned long lapDuration=0,bestLapDur=0,worstLapDur=0;
    volatile Ema sky,shadow; // Variable to store the EMA value 
    volatile bool shadowed=false;
       
    void setup(int pin){
      lightPin=pin;
      lastLapTime=millis();
    }

    /** This returns a value >=0 if lap happened*/
    int reportLap(){
      if(reportedLap<lapCounter){
        reportedLap++;
        return reportedLap;
      }
      return -1;
    }

  void detect(){
    // This is the interrupt handler
    unsigned long currentTime = millis();
    reading = analogRead(lightPin);
    if(!eyes){
      sky.learn(reading);
      return;
    }
    if(!shadowed){ //lit
        threshold=sky.learn(reading)/2;
        // Detect if the value is below the threshold and debounce logic
        if (reading < threshold && (currentTime - lastLapTime > minLapDuration)) { 
          //LAP DETECTED
          shadowed=true;
          if(lastLapTime==0){
            lapDuration=0; //race started crossed finish for first time          
          }else{
            lapDuration = currentTime-lastLapTime;
            if(lapDuration>worstLapDur || worstLapDur==0) worstLapDur=lapDuration;
            if(lapDuration<bestLapDur || bestLapDur==0) bestLapDur=lapDuration;
          }
          lastLapTime = currentTime;
          lapCounter++;
          //lap counter is now the number of completed laps
          if(lapCounter==raceLength){
              if(won){
                //didn't win :( 
              }else{
                winner=lightPin;
                won=true;
              }
          }
          // Car detected, perform your action here
          char fbuffer[10]; // Buffer to store the string representation of the float 
          dtostrf(sky.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
          char buffer[200];
          sprintf(buffer,"%d Car  threshold:%5d sky ema:%10s read:%5d lapCounter:%d lapDuration:%10lu\n",lightPin,threshold,fbuffer,reading,lapCounter,lapDuration);
          Serial.print(buffer);
        }           
    }else{      //shadowed
        shadow.learn(reading)/2;
        threshold=sky.get()/2+shadow.get()/2;
        if(reading>threshold){
          shadowed=false;
          // Car detected, perform your action here
          char fbuffer[10]; // Buffer to store the string representation of the float 
          dtostrf(shadow.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
          char buffer[200];
          sprintf(buffer,"%d LEFT threshold:%5d sha ema:%10s read:%5d lapCounter:%d lapDuration:%10lu\n",lightPin,threshold,fbuffer,reading,lapCounter,lapDuration);
          Serial.print(buffer);          
        }
    }    
  }  

  void display(){
    char buffer[100];
    lcd.setCursor(0,lightPin);//col,row
    char fbuffer[10];
    dtostrf(lapCounter>0 ? ((millis()-raceStart)/lapCounter)/1000.00  : 0, 2, 1, fbuffer); // Convert float to string: width=6, precision=2
    char ch=!won?'C' : winner==lightPin ? 'W' : 'L';    
    sprintf(buffer,"%c%d L%d %4lums %ss B:%lums W:%lums ",ch,lightPin,lapCounter,lapDuration,fbuffer,bestLapDur,worstLapDur);
    lcd.print(buffer);
  }
 



  
};
