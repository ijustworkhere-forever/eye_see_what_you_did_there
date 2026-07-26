#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "IRandomSource.h"

/**
 * Returns a pre-programmed sequence of values instead of real randomness;
 * the requested [min, max] range is recorded (`lastFloatMin/Max`,
 * `lastUIntMin/Max`) but not used to constrain the returned value — the
 * test author is responsible for programming sensible values. Cycles back
 * to the start if asked for more values than were provided.
 */
class FakeRandomSource : public eyesee::IRandomSource {
public:
    std::vector<float> floatSequence{0.0f};
    std::vector<uint32_t> uintSequence{0};
    float lastFloatMin = 0.0f;
    float lastFloatMax = 0.0f;
    uint32_t lastUIntMin = 0;
    uint32_t lastUIntMax = 0;

    float nextFloat(float min, float max) override {
        lastFloatMin = min;
        lastFloatMax = max;
        const float value = floatSequence[floatIndex_ % floatSequence.size()];
        ++floatIndex_;
        return value;
    }

    uint32_t nextUInt(uint32_t min, uint32_t max) override {
        lastUIntMin = min;
        lastUIntMax = max;
        const uint32_t value = uintSequence[uintIndex_ % uintSequence.size()];
        ++uintIndex_;
        return value;
    }

private:
    size_t floatIndex_ = 0;
    size_t uintIndex_ = 0;
};
