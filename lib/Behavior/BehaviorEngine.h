#pragma once

#include "CommandQueue.h"
#include "IAnimationEngine.h"
#include "IBehavior.h"
#include "IBehaviorEngine.h"

namespace eyesee {

/**
 * Drains CommandQueue every frame, dispatching each EyeCommand to
 * IAnimationEngine (never to EyeController directly), then delegates to
 * whichever IBehavior is active for the current EyeState.
 */
class BehaviorEngine : public IBehaviorEngine {
public:
    BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue, IBehavior& defaultBehavior);

    void setState(EyeState state) override;
    EyeState state() const override;
    void update(uint32_t deltaMs) override;

private:
    IAnimationEngine& animation_;
    CommandQueue& commandQueue_;
    IBehavior& activeBehavior_;
    EyeState state_;

    void dispatch(const EyeCommand& command);
};

}  // namespace eyesee
