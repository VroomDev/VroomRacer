/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
 struct Sensor {
  unsigned int acc=0; //to be set within ISR  
  unsigned long lastLapTime=0; //to be set within ISR
  float  n=0,mean=0,M2=0; //to be set within ISR
  
  unsigned int threshold=0; //to be set by loop
  unsigned int sd;
  



  void calcThreshold(){
    noInterrupts();    
    p("n",n);
    p("mean",mean);
    sd=sqrt(M2/n);
    p("sd",sd);
    threshold=mean-10*sd;
    pln("threshold",threshold);    
    interrupts();
  }
  
};
