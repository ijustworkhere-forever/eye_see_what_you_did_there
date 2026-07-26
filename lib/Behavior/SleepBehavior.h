#pragma once

#include "IBehavior.h"

namespace eyesee {

/**
 * Reports EyeState::Sleeping and otherwise stays inert. Closing the eyes is
 * NOT this class's job: BehaviorEngine::dispatch() calls
 * animation.animateSleep(command.durationMs) with the caller's requested
 * duration before it calls setState(Sleeping), so by the time this behavior
 * becomes active the eyes are already closing. Waking up is likewise
 * BehaviorEngine's job (docs/superpowers/specs/2026-07-25-v0.3-behavior-design.md).
 */
class SleepBehavior : public IBehavior {
public:
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;
};

}  // namespace eyesee
