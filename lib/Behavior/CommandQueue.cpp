#include "CommandQueue.h"

namespace eyesee {

#ifdef ARDUINO
namespace {
/** RAII critical-section guard around the ESP32 portMUX spinlock. */
class CriticalSection {
public:
    explicit CriticalSection(portMUX_TYPE& mux) : mux_(mux) { portENTER_CRITICAL(&mux_); }
    ~CriticalSection() { portEXIT_CRITICAL(&mux_); }

private:
    portMUX_TYPE& mux_;
};
}  // namespace
#define EYESEE_QUEUE_LOCK() CriticalSection lock(mux_)
#else
#define EYESEE_QUEUE_LOCK()
#endif

bool CommandQueue::push(const EyeCommand& command) {
    EYESEE_QUEUE_LOCK();
    if (count_ == kCapacity) {
        return false;
    }
    const size_t tail = (head_ + count_) % kCapacity;
    buffer_[tail] = command;
    ++count_;
    return true;
}

bool CommandQueue::pop(EyeCommand& outCommand) {
    EYESEE_QUEUE_LOCK();
    if (count_ == 0) {
        return false;
    }
    outCommand = buffer_[head_];
    head_ = (head_ + 1) % kCapacity;
    --count_;
    return true;
}

void CommandQueue::clear() {
    EYESEE_QUEUE_LOCK();
    head_ = 0;
    count_ = 0;
}

size_t CommandQueue::size() const {
    return count_;
}

bool CommandQueue::empty() const {
    return count_ == 0;
}

bool CommandQueue::full() const {
    return count_ == kCapacity;
}

}  // namespace eyesee
