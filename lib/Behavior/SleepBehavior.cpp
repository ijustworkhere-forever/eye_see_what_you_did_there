#include "SleepBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

void SleepBehavior::onEnter(IAnimationEngine& animation) {
    animation.animateSleep(kSleepCloseDurationMs);
}

void SleepBehavior::update(uint32_t deltaMs, IAnimationEngine& animation) {
    (void)deltaMs;
    (void)animation;
    // No-op: eyes stay shut until BehaviorEngine transitions away from Sleeping.
}

EyeState SleepBehavior::state() const {
    return EyeState::Sleeping;
}

}  // namespace eyesee
