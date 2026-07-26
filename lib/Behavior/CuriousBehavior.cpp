#include "CuriousBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

CuriousBehavior::CuriousBehavior(IRandomSource& randomSource)
    : randomSource_(randomSource), msUntilNextGlance_(0) {
}

void CuriousBehavior::onEnter(IAnimationEngine& animation) {
    animation.animateExpression(Expression::Curious, 200);
    msUntilNextGlance_ = randomInterval();
}

void CuriousBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
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

EyeState CuriousBehavior::state() const {
    return EyeState::Idle;
}

uint32_t CuriousBehavior::randomInterval() {
    return randomSource_.nextUInt(kMinIntervalMs, kMaxIntervalMs);
}

}  // namespace eyesee
