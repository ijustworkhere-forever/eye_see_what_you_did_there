#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "IRandomSource.h"

/**
 * Returns a pre-programmed sequence of values instead of real randomness.
 * Ignores the requested [min, max] range — the test author is responsible
 * for programming values that make sense for what's being tested. Cycles
 * back to the start if asked for more values than were provided.
 */
class FakeRandomSource : public eyesee::IRandomSource {
public:
    std::vector<float> floatSequence{0.0f};
    std::vector<uint32_t> uintSequence{0};

    float nextFloat(float min, float max) override {
        (void)min;
        (void)max;
        const float value = floatSequence[floatIndex_ % floatSequence.size()];
        ++floatIndex_;
        return value;
    }

    uint32_t nextUInt(uint32_t min, uint32_t max) override {
        (void)min;
        (void)max;
        const uint32_t value = uintSequence[uintIndex_ % uintSequence.size()];
        ++uintIndex_;
        return value;
    }

private:
    size_t floatIndex_ = 0;
    size_t uintIndex_ = 0;
};
