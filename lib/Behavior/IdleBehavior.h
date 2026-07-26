#pragma once

#include "GlanceBehavior.h"

namespace eyesee {

/**
 * Default resting behavior: periodically issues a small random gaze shift
 * (a micro-saccade), giving idle eyes a lifelike, non-static quality.
 */
class IdleBehavior : public GlanceBehavior {
public:
    explicit IdleBehavior(IRandomSource& randomSource);
};

}  // namespace eyesee
