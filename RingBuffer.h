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
  bool top(uint8_t index, T& item) const {
    noInterrupts();
    if (index >= count) {
      return false; // Index out of range
    }
    uint8_t pos = (head + Size - 1 - index) & (Size - 1); // Calculate the position
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
  bool peek(T& item) const {
    if (isEmpty()) {
      return false; // Buffer is empty
    }
    noInterrupts();
    item = buffer[tail]; // Peek at the item at the tail without modifying the buffer
    interrupts();
    return true;
  }
  
private:
  T buffer[Size];
  uint8_t head, tail, count;
};
