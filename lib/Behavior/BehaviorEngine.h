#pragma once

#include <array>
#include <cstddef>

#include "CommandQueue.h"
#include "IAnimationEngine.h"
#include "IBehavior.h"
#include "IBehaviorEngine.h"

namespace eyesee {

/**
 * Drains CommandQueue every frame, dispatching each EyeCommand to
 * IAnimationEngine (never to EyeController directly), then delegates to
 * whichever IBehavior is registered for the current EyeState (falling back
 * to a default behavior if none was registered for that state).
 */
class BehaviorEngine : public IBehaviorEngine {
public:
    BehaviorEngine(IAnimationEngine& animation, CommandQueue& commandQueue,
                   IBehavior& fallbackBehavior);

    /** Associates a behavior instance with a state. Call once per behavior during setup(). */
    void registerBehavior(EyeState state, IBehavior& behavior);

    void setState(EyeState state) override;
    EyeState state() const override;
    void update(uint32_t deltaMs) override;

private:
    // One slot per EyeState value (Startup..Error) — see EyeState.h.
    static constexpr size_t kEyeStateCount = 8;

    IAnimationEngine& animation_;
    CommandQueue& commandQueue_;
    IBehavior& fallbackBehavior_;
    std::array<IBehavior*, kEyeStateCount> behaviorsByState_{};
    IBehavior* activeBehavior_;
    EyeState state_;

    void dispatch(const EyeCommand& command);
    IBehavior& behaviorForState(EyeState state) const;
};

}  // namespace eyesee
