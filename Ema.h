

const float alpha = 2.0/(500+1); // Smoothing factor (0 < alpha <= 1)  
class Ema {
  public:
    
    volatile float value=-1;

    float learn(float reading){
      if(value<0) value=reading;
      else value = (alpha * reading) + ((1 - alpha) * value);
      return value;      
    }

    float get(){
      return value;
    }
     
};
