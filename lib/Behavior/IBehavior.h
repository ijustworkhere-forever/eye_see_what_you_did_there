#pragma once

#include <cstdint>

#include "EyeState.h"
#include "EyeTypes.h"

namespace eyesee {

class IAnimationEngine;  // forward declaration — only a reference is needed here

/** One pluggable behavior strategy (Idle, Tracking, Curious, Random, Sleep...). */
class IBehavior {
public:
    virtual ~IBehavior() = default;

    /** Called once when BehaviorEngine switches to this behavior. Default: no-op. */
    virtual void onEnter(IAnimationEngine& animation) {
        (void)animation;
    }
    /** Called once when BehaviorEngine switches away from this behavior. Default: no-op. */
    virtual void onExit(IAnimationEngine& animation) {
        (void)animation;
    }
    virtual void update(uint32_t deltaMs, IAnimationEngine& animation) = 0;
    virtual EyeState state() const = 0;

    /** Delivers a Track command's payload to whichever behavior is currently active.
     * Default no-op -- only TrackingBehavior (docs/ROADMAP.md v0.6) overrides this. */
    virtual void receiveGazeTarget(const GazeTarget& target) {
        (void)target;
    }
};

}  // namespace eyesee
