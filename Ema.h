/////////Ema by Chris Busch

const float alpha = 2.0/(500+1); // Smoothing factor (0 < alpha <= 1)  
class Ema {
  public:

    volatile int learns=0;
    volatile float value=-1;

    float learn(float reading){
      if(value<0) value=reading;
      else value = (alpha * reading) + ((1 - alpha) * value);
      if(learns<32000) learns++;
      return value;      
    }

    float get(){
      return value;
    }
     
};
