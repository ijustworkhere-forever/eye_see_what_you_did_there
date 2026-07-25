#include "BehaviorEngine.h"

namespace eyesee {

BehaviorEngine::BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue,
                                IBehavior& defaultBehavior)
    : animation_(animation),
      commandQueue_(commandQueue),
      activeBehavior_(defaultBehavior),
      state_(EyeState::Startup) {}

void BehaviorEngine::setState(EyeState state) {
    state_ = state;
    // TODO: swap activeBehavior_ based on state once more IBehavior
    // implementations exist (docs/ROADMAP.md v0.3). Single behavior this pass.
}

EyeState BehaviorEngine::state() const {
    return state_;
}

void BehaviorEngine::update(uint32_t deltaMs) {
    EyeCommand command;
    while (commandQueue_.pop(command)) {
        dispatch(command);
    }
    activeBehavior_.update(deltaMs, animation_);
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
            break;
        case CommandType::Wake:
            animation_.animateWake(command.durationMs);
            break;
        case CommandType::SetExpression:
            animation_.animateExpression(command.expression, command.durationMs);
            break;
    }
}

}  // namespace eyesee
