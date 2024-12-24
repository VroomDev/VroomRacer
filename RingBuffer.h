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

private:
  T buffer[Size];
  uint8_t head, tail, count;
};
