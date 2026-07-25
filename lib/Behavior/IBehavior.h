#pragma once

#include <cstdint>

#include "EyeState.h"

namespace eyesee {

class IAnimationEngine;  // forward declaration — only a reference is needed here

/** One pluggable behavior strategy (Idle, Tracking, Curious, Random, Sleep...). */
class IBehavior {
public:
    virtual ~IBehavior() = default;
    virtual void update(uint32_t deltaMs, IAnimationEngine& animation) = 0;
    virtual EyeState state() const = 0;
};

}  // namespace eyesee
