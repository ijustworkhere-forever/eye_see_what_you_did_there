#pragma once

#include "IBehavior.h"
#include "IRandomSource.h"

namespace eyesee {

/**
 * Shared countdown-glance state machine: on a random interval, issues one
 * animateGaze() call within a fixed range at a fixed speed, then resets the
 * countdown. IdleBehavior/RandomBehavior/CuriousBehavior differ only in
 * these four constants (and, for CuriousBehavior, an onEnter expression).
 */
class GlanceBehavior : public IBehavior {
public:
    GlanceBehavior(IRandomSource& randomSource, EyeState state, float glanceRange,
                   uint32_t minIntervalMs, uint32_t maxIntervalMs, float speedDegPerSec);

    void onEnter(IAnimationEngine& animation) override;
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    IRandomSource& randomSource_;
    EyeState state_;
    float glanceRange_;
    uint32_t minIntervalMs_;
    uint32_t maxIntervalMs_;
    float speedDegPerSec_;
    uint32_t msUntilNextGlance_;

    uint32_t randomInterval();
};

}  // namespace eyesee
