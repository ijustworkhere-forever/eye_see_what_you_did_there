#pragma once

#include "IBehavior.h"

namespace eyesee {

/** Closes the eyes once on entry and does nothing else — waking up is BehaviorEngine's job (docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md). */
class SleepBehavior : public IBehavior {
public:
    void onEnter(IAnimationEngine& animation) override;
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr uint32_t kSleepCloseDurationMs = 500;
};

}  // namespace eyesee
