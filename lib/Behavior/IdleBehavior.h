#pragma once

#include "IBehavior.h"
#include "IRandomSource.h"

namespace eyesee {

/**
 * Default resting behavior: periodically issues a small random gaze shift
 * (a micro-saccade), giving idle eyes a lifelike, non-static quality.
 */
class IdleBehavior : public IBehavior {
public:
    explicit IdleBehavior(IRandomSource& randomSource);

    void onEnter(IAnimationEngine& animation) override;
    void update(uint32_t deltaMs, IAnimationEngine& animation) override;
    EyeState state() const override;

private:
    static constexpr float kMicroSaccadeRange = 0.15f;  // normalized -0.15..0.15
    static constexpr uint32_t kMinIntervalMs = 500;
    static constexpr uint32_t kMaxIntervalMs = 2000;
    static constexpr float kMicroSaccadeSpeedDegPerSec = 300.0f;

    IRandomSource& randomSource_;
    uint32_t msUntilNextGlance_;

    uint32_t randomInterval();
};

}  // namespace eyesee
