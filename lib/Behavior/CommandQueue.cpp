#include "CommandQueue.h"

namespace eyesee {

bool CommandQueue::push(const EyeCommand& command) {
    if (full()) {
        return false;
    }
    const size_t tail = (head_ + count_) % kCapacity;
    buffer_[tail] = command;
    ++count_;
    return true;
}

bool CommandQueue::pop(EyeCommand& outCommand) {
    if (empty()) {
        return false;
    }
    outCommand = buffer_[head_];
    head_ = (head_ + 1) % kCapacity;
    --count_;
    return true;
}

void CommandQueue::clear() {
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
