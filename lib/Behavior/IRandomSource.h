#pragma once

#include <cstdint>

namespace eyesee {

/** Injectable randomness source so glance-based behaviors stay deterministic in tests. */
class IRandomSource {
public:
    virtual ~IRandomSource() = default;

    /** Uniform random float in [min, max]. */
    virtual float nextFloat(float min, float max) = 0;
    /** Uniform random integer in [min, max]. */
    virtual uint32_t nextUInt(uint32_t min, uint32_t max) = 0;
};

}  // namespace eyesee
