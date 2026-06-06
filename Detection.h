/*
   VroomRacer by Chris Busch (c) 2024
   There are no warranties express or implied with this code.
   No guarantees of being fit for purpose.
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
    RaceFlag flag;

    // Constructor to initialize the members
    Detection(uint8_t p, unsigned int v = 0, unsigned int c = 0, unsigned long t = 0, RaceFlag rf = FORMATION) : port(p), value(v), count(c), timestamp(t), flag(rf) {}

    Detection() : port(0), value(0), count(0), timestamp(0), flag(FORMATION) {}

    void set(uint8_t p, unsigned int v = 0, unsigned int c = 0, unsigned long t = 0, RaceFlag rf = FORMATION) {
      port = p; value = v; count = c; timestamp = t; flag = rf;
    }

    void reset() {
      port = 0;
      value = 0;
      count = 0;
      timestamp = 0;
      flag = FORMATION;
    }

    bool isEmpty() {
      return timestamp == 0 && value == 0 && port == 0 && count == 0;
    }

    void debug() {
      ph("Detection");
      p("millis", millis());
      p("port", port);
      p("value", value);
      p("count", count);
      p("flag", flag);
      pln("timestamp", timestamp);
    }

};


#endif // DETECTION_H
