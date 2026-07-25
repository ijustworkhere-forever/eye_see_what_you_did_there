#pragma once

#include "IBehavior.h"

namespace eyesee {

/**
 * The only IBehavior implementation this pass. TrackingBehavior,
 * CuriousBehavior, RandomBehavior, SleepBehavior are planned
 * (docs/ROADMAP.md v0.3) but not scaffolded as empty files yet.
 */
class IdleBehaviorStub : public IBehavior {
public:
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;
};

}  // namespace eyesee
