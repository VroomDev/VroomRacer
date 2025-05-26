/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
// a 2k resistor works better with LDR mean max is 145
// a 1k resistor with LDR mean max is 95

// The mininum time required for darkness to qualify as a car passing overhead the sensor.
// 2ms is 1250ips=71mph=4545 1:64mph, which is crazy fast, really isn't achievable
// 3ms is 883ips=47mph=3030 1:64mph
// 4ms is 625ips=36mph=2273 1:64mph, Tyco published 2000 scale mph was achievable, which is still crazy fast.
// May need consider higher min ms time
#define MIN_MS_TIME 2
//1 was used for drag racing
//2 was used for road course... i think 2 will work for both
#define MAXCOUNT 65535
const int DEBOUNCE = 100; // Debounce delay in milliseconds

struct Sensor {
  unsigned int acc=0; //to be set within ISR, this is the reading perhaps accumulated over a few reads
  unsigned long lastDetectTime=0; //to be set within ISR
    
  uint16_t minAcc=0,maxAcc=0;
  volatile uint16_t count=0;
    
  int initialThreshold = 200; // Example threshold value
  int mainThreshold = 128; // Example threshold value
  volatile bool darkEnough=false; //dark enough
  volatile bool longEnough=false;

  uint16_t n=0,mean=0;
  uint32_t total=0;
  uint16_t ticksPerMs=0;
  unsigned long startSampleTime=millis();
  unsigned long stopSampleTime=millis();

  void reset(){
    noInterrupts();        
    lastDetectTime=0;
    interrupts();
  }

  void debug(){
     ph("Sensor");
     p("acc",acc);
     p("lastDetectTime",lastDetectTime);
     p("minAcc",minAcc);
     p("maxAcc",maxAcc);
     p("count",count);
     p("initialThreshold",initialThreshold);
     p("mainThreshold",mainThreshold);
     p("mean",mean);
     p("ticks per ms", ticksPerMs );        
     pln("n",n);       
  }

  void display(int w){
    lcd.print("S:");
    lcd.print(w);
    lcd.print("R:");
    lcd.print(acc);
    lcd.print("IT:");
    lcd.print(initialThreshold);
    lcd.print("MT:");
    lcd.print(mainThreshold);
    lcd.print(" ");
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
    //Remember there can be some readings wiggle as the shadow of the car goes over.
    if (acc < initialThreshold) { //when below the threshold start monitoring
      if (count < MAXCOUNT) count++;
      if (acc < mainThreshold)  darkEnough = true;
      if (count>ticksPerMs*MIN_MS_TIME) longEnough=true;
    }    
    if (acc > initialThreshold) {
      //now that it is bright enough, was the event dark and long enough? If not, keep looking.
      if (darkEnough && longEnough) {
        auto currentTime=millis(); 
        if (currentTime > lastDetectTime + DEBOUNCE) {          
          //Detect detected       
          lastDetectTime = currentTime;          
          //put in ring buffer
          Detection detection(curSensor,acc,count,currentTime); // Initialize with data  
          ringBuffer.push(detection);          
        }
      }
      count=0;
      darkEnough=false;
      longEnough=false;
    }
  }


  void calcThreshold(){
    noInterrupts();        
    initialThreshold=mean*3/4;
    mainThreshold=mean/2;
    //initialThreshold=mean*4/5;
    //mainThreshold=mean*3/4;  
    ticksPerMs=n/(stopSampleTime-startSampleTime);
    ph("Sensor.calcThreshold");
    p("minAcc",minAcc);
    p("maxAcc",maxAcc);
    p("n",n);
    p("mean",mean);
    p("ticksPerMs",ticksPerMs);
    p("initialThreshold",initialThreshold);
    pln("mainThreshold",mainThreshold); 
    if(mean==0){
      playMusic(imperialMarchMelody,imperialMarchNotes,2*120); 
    }
    interrupts();
  }

 

  
};
