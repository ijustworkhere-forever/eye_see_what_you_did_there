#pragma once

#include <array>
#include <cstddef>

#ifdef ARDUINO
#include <freertos/FreeRTOS.h>
#endif

#include "EyeCommand.h"

namespace eyesee {

/**
 * Fixed-capacity FIFO ring buffer of EyeCommand. No heap allocation.
 *
 * Thread-safe on ESP32: push()/pop()/clear() are guarded by a FreeRTOS
 * critical section, since ESPAsyncWebServer's route handlers (which call
 * push()) run on AsyncTCP's own task, genuinely in parallel with the
 * Arduino loop() task that calls pop() via BehaviorEngine::update() on a
 * dual-core chip. Native tests are single-threaded, so the guard compiles
 * away to nothing there (no behavior change, no new test requirements).
 * size()/empty()/full() read a single size_t without locking -- an
 * aligned 32-bit read is atomic on this hardware, so these can't tear, but
 * a value read this way may already be stale by the time the caller acts
 * on it; that's an accepted, pre-existing characteristic of a "roughly
 * current size" query, not something this fix changes.
 */
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
#ifdef ARDUINO
    portMUX_TYPE mux_ = portMUX_INITIALIZER_UNLOCKED;
#endif
};

}  // namespace eyesee
