/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */

//define BIT8 to have 8 bit sensors
//#define BIT8

typedef enum : uint8_t { PRESAMPLE,  GO, FINISH } IsrFlag; 
volatile IsrFlag isrFlag = PRESAMPLE;

#define NUMSENSORS 2
static_assert((NUMSENSORS & (NUMSENSORS - 1)) == 0, "NUMSENSORS must be a power of two");
//ACCSMOOTH must be base 2
#define ACCSMOOTH 8
static_assert((ACCSMOOTH & (ACCSMOOTH - 1)) == 0, "ACCSMOOTH must be a power of two");

Sensor sensors[NUMSENSORS];

class ISR{
  public:
  
  static setup(){
    ADCSRA = 0;             // clear ADCSRA register
    ADCSRB = 0;             // clear ADCSRB register
    ADMUX = (ADMUX & 0xF0) | 0x00; // We will start with pin A0
    ADMUX |= (1 << REFS0);  // set reference voltage

    #ifdef BIT8
    ADMUX |= (1 << ADLAR);  // left align ADC value to 8 bits from ADCH register
    #endif
  
    // sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
    // for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles
    //ADCSRA |= (1 << ADPS2) | (1 << ADPS0);    // 32 prescaler for 38.5 KHz
    ADCSRA |= (1 << ADPS2);                     // 16 prescaler for 76.9 KHz
    //ADCSRA |= (1 << ADPS1) | (1 << ADPS0);    // 8 prescaler for 153.8 KHz
        
    ADCSRA |= (1 << ADIF); // Clear ADIF by writing a 1 to it (this is how you clear the flag)
   
    ADCSRA |= (1 << ADATE); // enable auto trigger
    ADCSRA |= (1 << ADIE);  // enable interrupts when measurement complete
    ADCSRA |= (1 << ADEN);  // enable ADC
    ADCSRA |= (1 << ADSC);  // start ADC measurements
  }

  static void calcThresholds(){
    for(int i=0;i<NUMSENSORS;i++){
      Sensor& s=sensors[i];
      p("i",i);
      s.calcThreshold();
    }
  }

  static void go(){
    isrFlag=GO;
  }
  
};


uint8_t curSensor = 0;
volatile int pings=0;
volatile unsigned int notReady=0;


//// ADC interrupt service routine
ISR(ADC_vect) {
  static uint8_t accCount=0; 
  Sensor& sensor=sensors[curSensor];
  auto nextSensor=(curSensor + 1) & (NUMSENSORS-1);
  if(accCount==0){
    sensor.acc=0;
  }
  while(!(ADCSRA & (1 << ADIF))) { //wait
    //spin wheels    
  }
  ADCSRA |= (1 << ADIF); // Clear ADIF by writing a 1 to it (this is how you clear the flag)
  #ifdef BIT8
    sensor.acc += ADCH;   // Read the ADC value for 8 bit resolution
  #else
    sensor.acc += ADC; // 10 bit resolution
  #endif
  ADMUX = (ADMUX & 0xF0) | nextSensor;  // Update to the next channel
  if(accCount==ACCSMOOTH-1){ //we accumulated enough readings
    //now do the checking on this value
    auto flag=isrFlag;
    if(flag==PRESAMPLE) {
      sensor.presample();      
    }else if(flag==GO){
        if(sensor.acc < sensor.threshold){     
           auto currentTime=millis();        
           if(currentTime - sensor.lastLapTime > minLapDuration) { 
              //lap detected
              sensor.lastLapTime=currentTime;
              //put in ring buffer
              Detection detection(curSensor,sensor.acc,sensor.lastLapTime); // Initialize with data  
              ringBuffer.push(detection);
           }
        }
    }      
  }
  if(curSensor==NUMSENSORS-1){
    accCount=(accCount+1) & (ACCSMOOTH-1);    
  }
  curSensor = nextSensor;
}