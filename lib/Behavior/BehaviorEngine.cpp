#include "BehaviorEngine.h"

namespace eyesee {

// Guards BehaviorEngine::kEyeStateCount (private, see BehaviorEngine.h) against silently
// going stale if EyeState grows without updating it — registerBehavior()/behaviorForState()
// index behaviorsByState_ with a raw static_cast<size_t>(state) and would go out-of-bounds
// with no compiler error and no test failure otherwise.
static_assert(static_cast<size_t>(EyeState::Error) == 7,
              "EyeState grew — update BehaviorEngine::kEyeStateCount in BehaviorEngine.h to match");

BehaviorEngine::BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue,
                               IBehavior& fallbackBehavior)
    : animation_(animation),
      commandQueue_(commandQueue),
      fallbackBehavior_(fallbackBehavior),
      activeBehavior_(nullptr),
      state_(EyeState::Startup) {
}

void BehaviorEngine::registerBehavior(EyeState state, IBehavior& behavior) {
    behaviorsByState_[static_cast<size_t>(state)] = &behavior;
}

void BehaviorEngine::setState(EyeState state) {
    IBehavior& next = behaviorForState(state);
    if (activeBehavior_ != &next) {
        // activeBehavior_ starts null (see constructor) so the very first transition always
        // fires onEnter() exactly once, regardless of which instance it resolves to.
        if (activeBehavior_ != nullptr) {
            activeBehavior_->onExit(animation_);
        }
        activeBehavior_ = &next;
        activeBehavior_->onEnter(animation_);
    }
    state_ = state;
}

EyeState BehaviorEngine::state() const {
    return state_;
}

void BehaviorEngine::update(uint32_t deltaMs) {
    EyeCommand command;
    while (commandQueue_.pop(command)) {
        dispatch(command);
    }
    if (activeBehavior_ != nullptr) {
        activeBehavior_->update(deltaMs, animation_);
    }
}

void BehaviorEngine::dispatch(const EyeCommand& command) {
    switch (command.type) {
        case CommandType::Look:
            animation_.animateGaze(command.gazeTarget);
            break;
        case CommandType::Blink:
            animation_.animateBlink(command.durationMs);
            break;
        case CommandType::WinkLeft:
            animation_.animateWinkLeft(command.durationMs);
            break;
        case CommandType::WinkRight:
            animation_.animateWinkRight(command.durationMs);
            break;
        case CommandType::Sleep:
            animation_.animateSleep(command.durationMs);
            setState(EyeState::Sleeping);
            break;
        case CommandType::Wake:
            animation_.animateWake(command.durationMs);
            setState(EyeState::Idle);
            break;
        case CommandType::SetExpression:
            animation_.animateExpression(command.expression, command.durationMs);
            break;
    }
}

IBehavior& BehaviorEngine::behaviorForState(EyeState state) const {
    IBehavior* registered = behaviorsByState_[static_cast<size_t>(state)];
    return registered != nullptr ? *registered : fallbackBehavior_;
}

}  // namespace eyesee
