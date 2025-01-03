/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */

 #define MAXCOUNT 65535
 struct Sensor {
  unsigned int acc=0; //to be set within ISR, this is the reading perhaps accumulated over a few reads
  unsigned long lastLapTime=0; //to be set within ISR
    
  unsigned int minAcc=0,maxAcc=0,count=0;
    
  int initialThreshold = 200; // Example threshold value
  int mainThreshold = 128; // Example threshold value
  bool qualified=false;

  uint16_t n=0,mean=0;
  uint32_t total=0;
  uint16_t ticksPerMs=0;
  unsigned long startSampleTime=millis();
  unsigned long stopSampleTime=millis();

  void debug(){
     ph("Sensor");
     p("acc",acc);
     p("lastLapTime",lastLapTime);
     p("minAcc",minAcc);
     p("maxAcc",maxAcc);
     p("count",count);
     p("initialThreshold",initialThreshold);
     p("mainThreshold",mainThreshold);
     p("ticks per ms", ticksPerMs );        
     pln("n",n);       
  }
    
  //called by the ISR
  void presample(){
    if(n<65535){
      if(n==0) startSampleTime=millis();
      n++;          
      if(n<1024){          
          total+=acc;
          mean=total/n;
      }
      stopSampleTime=millis();
    }
    if(acc>maxAcc) maxAcc=acc;
    if(acc<minAcc || minAcc==0) minAcc=acc;
  }

  void go(int curSensor){
    if (acc < initialThreshold) {
      if (count < MAXCOUNT) count++;
      if (acc < mainThreshold)  qualified = true;
    }    
    if (acc >= initialThreshold) {
      if (qualified) {
        auto currentTime=millis(); 
        if (currentTime > lastLapTime + minLapDuration) {          
          //lap detected       
          lastLapTime = currentTime;          
          //put in ring buffer
          Detection detection(curSensor,acc,count,lastLapTime); // Initialize with data  
          ringBuffer.push(detection);          
        }
        qualified = false;
        count = 0;
      }
    }
  }


  void calcThreshold(){
    noInterrupts();        
    initialThreshold=mean*3/4;
    mainThreshold=mean/2;  
    ticksPerMs=n/(stopSampleTime-startSampleTime);
    ph("Sensor.calcThreshold");
    p("minAcc",minAcc);
    p("maxAcc",maxAcc);
    p("n",n);
    p("mean",mean);
    p("ticksPerMs",ticksPerMs);
    p("initialThreshold",initialThreshold);
    pln("mainThreshold",mainThreshold);  
    interrupts();
  }

 

  
};
