#include "GlanceBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

GlanceBehavior::GlanceBehavior(IRandomSource& randomSource, EyeState state, float glanceRange,
                               uint32_t minIntervalMs, uint32_t maxIntervalMs,
                               float speedDegPerSec)
    : randomSource_(randomSource),
      state_(state),
      glanceRange_(glanceRange),
      minIntervalMs_(minIntervalMs),
      maxIntervalMs_(maxIntervalMs),
      speedDegPerSec_(speedDegPerSec),
      msUntilNextGlance_(0) {
}

void GlanceBehavior::onEnter(IAnimationEngine& animation) {
    (void)animation;
    msUntilNextGlance_ = randomInterval();  // settle first; don't glance the instant this begins
}

void GlanceBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    if (deltaMs >= msUntilNextGlance_) {
        GazeTarget target;
        target.x = randomSource_.nextFloat(-glanceRange_, glanceRange_);
        target.y = randomSource_.nextFloat(-glanceRange_, glanceRange_);
        target.speed = speedDegPerSec_;
        animation.animateGaze(target);
        msUntilNextGlance_ = randomInterval();
    } else {
        msUntilNextGlance_ -= deltaMs;
    }
}

EyeState GlanceBehavior::state() const {
    return state_;
}

uint32_t GlanceBehavior::randomInterval() {
    return randomSource_.nextUInt(minIntervalMs_, maxIntervalMs_);
}

}  // namespace eyesee
