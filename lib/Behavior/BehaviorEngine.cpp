#include "BehaviorEngine.h"

namespace eyesee {

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
