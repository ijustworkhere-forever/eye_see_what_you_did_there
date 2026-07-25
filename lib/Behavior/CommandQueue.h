#pragma once

#include <array>
#include <cstddef>

#include "EyeCommand.h"

namespace eyesee {

/** Fixed-capacity FIFO ring buffer of EyeCommand. No heap allocation. */
class CommandQueue {
public:
    static constexpr size_t kCapacity = 16;

    bool push(const EyeCommand& command);
    bool pop(EyeCommand& outCommand);
    void clear();

    size_t size() const;
    bool empty() const;
    bool full() const;

private:
    std::array<EyeCommand, kCapacity> buffer_{};
    size_t head_ = 0;
    size_t count_ = 0;
};

}  // namespace eyesee
