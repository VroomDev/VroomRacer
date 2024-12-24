//
class Detection {
  public:
  uint8_t port;
  unsigned long value;
  unsigned long timestamp;

  // Constructor to initialize the members
  Detection(uint8_t p,unsigned long v = 0, unsigned long t = 0) : port(p), value(v), timestamp(t) {}

  Detection() : port(0), value(0), timestamp(0) {}

  bool isEmpty(){
    return timestamp==0 && value==0 && port==0;
  }

  void debug(){
    p("port",port);
    p("value",value);
    p("timestamp",timestamp);
  }
  
};
