#include "IdleBehavior.h"

namespace eyesee {

namespace {
constexpr float kMicroSaccadeRange = 0.15f;  // normalized -0.15..0.15
constexpr uint32_t kMinIntervalMs = 500;
constexpr uint32_t kMaxIntervalMs = 2000;
constexpr float kMicroSaccadeSpeedDegPerSec = 300.0f;
}  // namespace

IdleBehavior::IdleBehavior(IRandomSource& randomSource)
    : GlanceBehavior(randomSource, EyeState::Idle, kMicroSaccadeRange, kMinIntervalMs,
                     kMaxIntervalMs, kMicroSaccadeSpeedDegPerSec) {
}

}  // namespace eyesee
