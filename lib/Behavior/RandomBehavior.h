#pragma once

#include "IBehavior.h"
#include "IRandomSource.h"

namespace eyesee {

/**
 * An erratic idle personality: frequent, wide, fast glances. Built and
 * tested this pass but not registered in main.cpp's EyeState::Idle slot
 * yet — see CuriousBehavior's header comment for why.
 */
class RandomBehavior : public IBehavior {
public:
    explicit RandomBehavior(IRandomSource& randomSource);

    void onEnter(IAnimationEngine& animation) override;
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr float kGlanceRange = 0.9f;
    static constexpr uint32_t kMinIntervalMs = 200;
    static constexpr uint32_t kMaxIntervalMs = 800;
    static constexpr float kGlanceSpeedDegPerSec = 600.0f;

    IRandomSource& randomSource_;
    uint32_t msUntilNextGlance_;

    uint32_t randomInterval();
};

}  // namespace eyesee
