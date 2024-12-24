/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
 struct Sensor {
  unsigned int acc=0; //to be set within ISR  
  unsigned long lastLapTime=0; //to be set within ISR
  float  n=0,mean=0,M2=0; //to be set within ISR

  unsigned int minAcc=0,maxAcc=0;
  unsigned int threshold=0; //to be set by loop
   float sd;
  
  void presample(){
    if(n<100){
          n += 1;
          float delta = acc - mean;
          mean += delta/n;
          M2 += delta*(acc - mean);
    }
    if(acc>maxAcc) maxAcc=acc;
    if(acc<minAcc || minAcc==0) minAcc=acc;
  }


  void calcThreshold(){
    noInterrupts();    
    p("minAcc",minAcc);
    p("maxAcc",maxAcc);
    p("n",n);
    p("mean",mean);
    sd=sqrt(M2/n);
    p("sd",sd);
    threshold=(mean-sd-1)/2;
//    if(minAcc<threshold) threshold=minAcc-1;
    pln("threshold",threshold);    
    interrupts();
  }
  
};
