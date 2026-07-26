#pragma once

#include "IBehavior.h"
#include "IRandomSource.h"

namespace eyesee {

/**
 * An alternative idle personality: wider, more deliberate glances than
 * IdleBehavior's micro-saccades, paired with a Curious expression. Built
 * and tested this pass but not registered in main.cpp's EyeState::Idle
 * slot yet (docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md) —
 * a future config/REST choice, not a new EyeState.
 */
class CuriousBehavior : public IBehavior {
public:
    explicit CuriousBehavior(IRandomSource& randomSource);

    void onEnter(IAnimationEngine& animation) override;
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr float kGlanceRange = 0.6f;             // wider than IdleBehavior's 0.15
    static constexpr uint32_t kMinIntervalMs = 1500;
    static constexpr uint32_t kMaxIntervalMs = 4000;
    static constexpr float kGlanceSpeedDegPerSec = 200.0f;  // slower, more deliberate

    IRandomSource& randomSource_;
    uint32_t msUntilNextGlance_;

    uint32_t randomInterval();
};

}  // namespace eyesee
