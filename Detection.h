/*
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 //
#ifndef DETECTION_H
#define DETECTION_H


 
class Detection {
  public:
  uint8_t port;
  unsigned int value;
  unsigned int count;
  unsigned long timestamp;

  // Constructor to initialize the members
  Detection(uint8_t p,unsigned int v = 0,unsigned int c=0, unsigned long t = 0) : port(p), value(v),count(c), timestamp(t) {}

  Detection() : port(0), value(0), count(0), timestamp(0) {}

  void set(uint8_t p,unsigned int v = 0,unsigned int c=0, unsigned long t = 0) {
     port=p; value=v; count=c; timestamp=t;
  }

  void reset(){
    port=0;
    value=0;
    count=0;
    timestamp=0;
  }

  bool isEmpty(){
    return timestamp==0 && value==0 && port==0 && count==0;
  }

  void debug(){
    ph("Detection");
    p("port",port);
    p("value",value);
    p("count",count);    
    pln("timestamp",timestamp);
  }
  
};


#endif // DETECTION_H
