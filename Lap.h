/* 
 * VroomRacer by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */

/**
 * This is used to store recent laps
 */
struct Lap {
  long int duration = 0;
  uint8_t lap = 0;

  // Primary comparison: shorter duration; tie-breaker: lower lap number
  bool operator<(const Lap& other) const {
    return duration < other.duration || 
          (duration == other.duration && lap < other.lap);
  }

  bool operator>(const Lap& other) const {
    return duration > other.duration || 
          (duration == other.duration && lap > other.lap);
  }

  bool operator<=(const Lap& other) const {
    return !(other < *this);
  }

  bool operator>=(const Lap& other) const {
    return !(*this < other);
  }

  bool operator==(const Lap& other) const {
    return duration == other.duration && lap == other.lap;
  }

  bool operator!=(const Lap& other) const {
    return !(*this == other);
  }
};
