#include "RandomBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

RandomBehavior::RandomBehavior(IRandomSource& randomSource)
    : randomSource_(randomSource), msUntilNextGlance_(0) {
}

void RandomBehavior::onEnter(IAnimationEngine& animation) {
    (void)animation;
    msUntilNextGlance_ = randomInterval();
}

void RandomBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    if (deltaMs >= msUntilNextGlance_) {
        GazeTarget target;
        target.x = randomSource_.nextFloat(-kGlanceRange, kGlanceRange);
        target.y = randomSource_.nextFloat(-kGlanceRange, kGlanceRange);
        target.speed = kGlanceSpeedDegPerSec;
        animation.animateGaze(target);
        msUntilNextGlance_ = randomInterval();
    } else {
        msUntilNextGlance_ -= deltaMs;
    }
}

EyeState RandomBehavior::state() const {
    return EyeState::Idle;
}

uint32_t RandomBehavior::randomInterval() {
    return randomSource_.nextUInt(kMinIntervalMs, kMaxIntervalMs);
}

}  // namespace eyesee
