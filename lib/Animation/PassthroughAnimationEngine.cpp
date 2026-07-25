#include "PassthroughAnimationEngine.h"

namespace eyesee {

PassthroughAnimationEngine::PassthroughAnimationEngine(EyeController& eyeController)
    : eyeController_(eyeController) {}

void PassthroughAnimationEngine::animateGaze(const GazeTarget& target) {
    // TODO: honor target.speed/blinkOnArrival/hold via real interpolation (docs/ROADMAP.md v0.2).
    eyeController_.look(target.x, target.y);
}

void PassthroughAnimationEngine::animateBlink(uint32_t durationMs) {
    (void)durationMs;  // TODO: reopen after durationMs (docs/ROADMAP.md v0.3)
    eyeController_.blink();
}

void PassthroughAnimationEngine::animateWinkLeft(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.winkLeft();
}

void PassthroughAnimationEngine::animateWinkRight(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.winkRight();
}

void PassthroughAnimationEngine::animateSleep(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.sleep();
}

void PassthroughAnimationEngine::animateWake(uint32_t durationMs) {
    (void)durationMs;
    eyeController_.wake();
}

void PassthroughAnimationEngine::animateExpression(Expression expression, uint32_t durationMs) {
    (void)durationMs;
    eyeController_.setExpression(expression);
}

void PassthroughAnimationEngine::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved — nothing in-progress to advance until real interpolation exists.
}

}  // namespace eyesee
