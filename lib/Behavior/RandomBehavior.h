#pragma once

#include "GlanceBehavior.h"

namespace eyesee {

/**
 * An erratic idle personality: frequent, wide, fast glances. Built and
 * tested this pass but not registered in main.cpp's EyeState::Idle slot
 * yet — see CuriousBehavior's header comment for why.
 */
class RandomBehavior : public GlanceBehavior {
public:
    explicit RandomBehavior(IRandomSource& randomSource);
};

}  // namespace eyesee
