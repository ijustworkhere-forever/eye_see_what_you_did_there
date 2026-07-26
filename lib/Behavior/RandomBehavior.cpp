#include "RandomBehavior.h"

namespace eyesee {

namespace {
constexpr float kGlanceRange = 0.9f;
constexpr uint32_t kMinIntervalMs = 200;
constexpr uint32_t kMaxIntervalMs = 800;
constexpr float kGlanceSpeedDegPerSec = 600.0f;
}  // namespace

RandomBehavior::RandomBehavior(IRandomSource& randomSource)
    : GlanceBehavior(randomSource, EyeState::Idle, kGlanceRange, kMinIntervalMs, kMaxIntervalMs,
                     kGlanceSpeedDegPerSec) {
}

}  // namespace eyesee
