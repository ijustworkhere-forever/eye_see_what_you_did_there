#include "IdleBehaviorStub.h"

#include "IAnimationEngine.h"

namespace eyesee {

void IdleBehaviorStub::update(uint32_t deltaMs, IAnimationEngine& animation) {
    (void)deltaMs;
    (void)animation;
    // TODO: idle scanning / micro-saccades (docs/ROADMAP.md v0.3). No-op this pass.
}

EyeState IdleBehaviorStub::state() const {
    return EyeState::Idle;
}

}  // namespace eyesee
