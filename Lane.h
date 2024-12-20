////////////////////////Lane.h by Chris Busch

const int minLapDuration = 500; // Debounce delay in milliseconds

unsigned int tock=0;

class Lane {
  static const bool serialOn=true;
  public:
    int lightPin = 0;
    volatile int threshold = 500; // Threshold value for detection
    volatile unsigned long lastLapTime = 0; //millis(); // Time the last valid detection occurred
  
    volatile int reading=0;
    volatile int lapCounter=-1;
    int reportedLap=-1;
    float sunsetDiv=1.6;
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
    reading = analogRead(lightPin);
    if(reading<2) return; //bogus reading
    if(tock++<1000 && reading>0){
//        if(serialOn) Serial.print(reading);
//        if(serialOn) Serial.print("\n");
    }
    if(!eyes){
      sky.learn(reading);
      return;
    }
    if(shadowed){       //shadowed
        if( shadow.get()<0) shadow.learn(reading);
        threshold=sky.get()/2+shadow.get()/2; //dawn
        sunsetDiv=((sky.get()/threshold)+sunsetDiv)/2;
        if(reading>threshold){
          shadowed=false;
          if(serialOn){
            // Car detected, perform your action here
            char fbuffer[10]; // Buffer to store the string representation of the float 
            dtostrf(shadow.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
            char buffer[200];
            sprintf(buffer,"C%d LEFT dawnt:%d shaEMA:%s read:%d lapCounter:%d lapDuration:%lu sunsetDiv=%d nightlearns=%d\n",
              lightPin,threshold,fbuffer,reading,lapCounter,lapDuration,(int)(sunsetDiv*1000),(int)shadow.learns);
            if(serialOn) Serial.print(buffer);
          }
        }else{
          shadow.learn(reading);
        }
    }else{//daylight
        sunsetDiv=1.3;// 0.7692
        threshold=sky.value/sunsetDiv; //sunset threshold
        // Detect if the value is below the threshold and debounce logic
        if( reading>=threshold){ //daylight
          sky.learn(reading);
        }else{ //darkness
          unsigned long currentTime = millis();
          shadow.learn(reading);
          if(currentTime - lastLapTime > minLapDuration) { 
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
            if(serialOn){
              char fbuffer[10]; // Buffer to store the string representation of the float 
              dtostrf(sky.get(), 6, 2, fbuffer); // Convert float to string: width=6, precision=2
              char buffer[200];
              sprintf(buffer,"C%d sunsetT:%d skyema:%s read:%d lapCounter:%d lapDuration:%lu daylearns:%d\n",
                lightPin,threshold,fbuffer,reading,lapCounter,lapDuration,(int)sky.learns);
              if(serialOn) Serial.print(buffer); //DAYLIGHT
            }
          } 
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
