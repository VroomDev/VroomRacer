/* 
 * Code by Chris Busch (c) 2024
 * There are no warranties express or implied with this code.
 * No guarantees of being fit for purpose.
 */
 
 template <typename T, uint8_t Size>
class RingBuffer {
  static_assert((Size & (Size - 1)) == 0, "Size must be a power of two");
  static_assert(Size <= 128, "max Size is 128");
  static_assert(Size > 0, "need Size>0");

public:
  RingBuffer() : head(0), tail(0), count(0) {}

  // only the ISR is allowed to call
  inline bool isFull() const {
    uint8_t c = count;
    return c == Size;
  }

  inline bool isEmpty() const {
    noInterrupts();
    uint8_t c = count;
    interrupts();
    return c == 0;
  }

  // only the ISR is allowed to call
  bool push(const T& item) {
    if (isFull()) {
      return false; // Buffer is full
    }
    buffer[head] = item;
    head = (head + 1) & (Size - 1); // Efficient modulo operation
    count++;
    return true;
  }

  bool pushAlways(const T& item) {
    // If buffer is full, increment tail to drop the oldest item
    if (isFull()) {
      tail = (tail + 1) & (Size - 1); // Efficient modulo operation
      count--; // Decrement count to allow new item
    }
    // Push the new item
    return push(item);
  }



  bool pull(T& item) {
    if (isEmpty()) {
      return false; // Buffer is empty
    }
    // Disable interrupts to ensure safe access
    noInterrupts();
    item = buffer[tail];
    tail = (tail + 1) & (Size - 1); // Efficient modulo operation
    count--;
    // Re-enable interrupts
    interrupts();
    return true;
  }

    /**
   * @brief Peek at the most recently pushed item in the buffer without removing it.
   * 
   * The index is relative to the most recent item pushed. For example, top(0) returns
   * the most recent item, top(1) returns the second most recent item, and so on.
   * 
   * @param index The index relative to the most recent item.
   * @param item The variable to store the peeked item.
   * @return true if the item was successfully peeked, false if the index is out of range.
   */
  bool top(T& item,uint8_t index=0) const {
    noInterrupts();
    if (index >= count) {
      interrupts();
      return false; // Index out of range
    }
    uint8_t pos = (head + Size - 1 - index) & (Size - 1); // Calculate the position
    item = buffer[pos];
    interrupts();
    return true;
  }

  /**
   * @brief Peek at the oldest item in the buffer without removing it.
   * 
   * The index is relative to the oldest item pushed. For example, bottom(0) returns
   * the oldest item (lowest lap time if buffer is sorted), bottom(1) returns the second-oldest, etc.
   * 
   * @param index The index relative to the oldest item.
   * @param item The variable to store the peeked item.
   * @return true if the item was successfully peeked, false if the index is out of range.
   */
  bool bottom(T& item, uint8_t index = 0) const {
    noInterrupts();
    if (index >= count) {
      interrupts();
      return false; // Index out of range
    }
    uint8_t pos = (tail + index) & (Size - 1); // Start from tail and move forward
    item = buffer[pos];
    interrupts();
    return true;
  }


  /**
   * @brief Peek at an item in the buffer without removing it.
   * 
   * This function allows you to peek at the next item in the buffer without removing it.
   * The item is copied to the provided variable, but the buffer state remains unchanged.
   * 
   * @param item The variable to store the peeked item.
   * @return true if the item was successfully peeked, false if the buffer is empty.
   */
  bool peek(T& item, uint8_t offset = 0) const {
    noInterrupts();
    if (isEmpty() || offset >= count) {
      interrupts();
      return false; // Buffer is empty or offset is too large
    }
    uint8_t pos = (tail + offset) & (Size - 1); // Calculate the position with offset
    item = buffer[pos]; // Peek at the item at the calculated position
    interrupts();
    return true;
  }

  void empty() {
    T d;
    while( pull(d)) {
        playTone(450,10); //empty buffer
        ph("Pulled");
    }
  }


   /**
   * pushSort keeps the lowest duration laps.
   * Not interrupt safe.  Do not use this on any RingBuffer that is used during interupts.
   */
   bool pushSort(const T& item) {
    //noInterrupts();
  
    // Case 1: Buffer is not full – just insert at the correct position
    if (!isFull()) {
      uint8_t i = head;
      // Find insertion point from tail to head (ascending order)
      for (uint8_t idx = 0; idx < count; idx++) {
        uint8_t pos = (tail + idx) & (Size - 1);
        if (item < buffer[pos]) {
          i = pos;
          break;
        }
      }
  
      // Shift elements right to make space
      uint8_t insertPos = i;
      uint8_t cur = head;
      while (cur != insertPos) {
        uint8_t prev = (cur - 1 + Size) & (Size - 1);
        buffer[cur] = buffer[prev];
        cur = prev;
      }
  
      buffer[insertPos] = item;
      head = (head + 1) & (Size - 1);
      count++;
      //interrupts();
      return true;
    }
  
    // Case 2: Buffer is full – check if item is smaller than largest
    uint8_t maxPos = (head - 1 + Size) & (Size - 1);
    if (item >= buffer[maxPos]) {
      //interrupts();
      return false; // Item too large
    }
  
    // Drop largest item and insert new one in correct place
    uint8_t i = tail;
    for (uint8_t idx = 0; idx < count; idx++) {
      uint8_t pos = (tail + idx) & (Size - 1);
      if (item < buffer[pos]) {
        i = pos;
        break;
      }
    }
  
    // Shift elements right from maxPos to insertion point
    uint8_t cur = maxPos;
    while (cur != i) {
      uint8_t prev = (cur - 1 + Size) & (Size - 1);
      buffer[cur] = buffer[prev];
      cur = prev;
    }
  
    buffer[i] = item;
    head = (head + 1) & (Size - 1);
    tail = (tail + 1) & (Size - 1); // Drop oldest to maintain size
    //interrupts();
    return true;
  }

  
private:
  T buffer[Size];
  uint8_t head, tail, count;
};
