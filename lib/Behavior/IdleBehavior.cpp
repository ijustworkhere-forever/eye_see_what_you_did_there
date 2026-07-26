#include "IdleBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

IdleBehavior::IdleBehavior(IRandomSource& randomSource)
    : randomSource_(randomSource), msUntilNextGlance_(0) {
}

void IdleBehavior::onEnter(IAnimationEngine& animation) {
    (void)animation;
    msUntilNextGlance_ = randomInterval();  // settle first; don't glance the instant idle begins
}

void IdleBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    if (deltaMs >= msUntilNextGlance_) {
        GazeTarget target;
        target.x = randomSource_.nextFloat(-kMicroSaccadeRange, kMicroSaccadeRange);
        target.y = randomSource_.nextFloat(-kMicroSaccadeRange, kMicroSaccadeRange);
        target.speed = kMicroSaccadeSpeedDegPerSec;
        animation.animateGaze(target);
        msUntilNextGlance_ = randomInterval();
    } else {
        msUntilNextGlance_ -= deltaMs;
    }
}

EyeState IdleBehavior::state() const {
    return EyeState::Idle;
}

uint32_t IdleBehavior::randomInterval() {
    return randomSource_.nextUInt(kMinIntervalMs, kMaxIntervalMs);
}

}  // namespace eyesee
