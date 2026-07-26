#include "CuriousBehavior.h"

#include "IAnimationEngine.h"

namespace eyesee {

namespace {
constexpr float kGlanceRange = 0.6f;  // wider than IdleBehavior's 0.15
constexpr uint32_t kMinIntervalMs = 1500;
constexpr uint32_t kMaxIntervalMs = 4000;
constexpr float kGlanceSpeedDegPerSec = 200.0f;  // slower, more deliberate
}  // namespace

CuriousBehavior::CuriousBehavior(IRandomSource& randomSource)
    : GlanceBehavior(randomSource, EyeState::Idle, kGlanceRange, kMinIntervalMs, kMaxIntervalMs,
                     kGlanceSpeedDegPerSec) {
}

void CuriousBehavior::onEnter(IAnimationEngine& animation) {
    animation.animateExpression(Expression::Curious, 200);
    GlanceBehavior::onEnter(animation);
}

}  // namespace eyesee
