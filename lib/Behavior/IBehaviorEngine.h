#pragma once

#include <cstdint>

#include "EyeState.h"

namespace eyesee {

class IBehaviorEngine {
public:
    virtual ~IBehaviorEngine() = default;
    virtual void setState(EyeState state) = 0;
    virtual EyeState state() const = 0;
    virtual void update(uint32_t deltaMs) = 0;
};

}  // namespace eyesee
