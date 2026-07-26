#pragma once

#include "IBehavior.h"

/** A test double IBehavior that records lifecycle/update call counts and reports a fixed state. */
class FakeBehavior : public eyesee::IBehavior {
public:
    explicit FakeBehavior(eyesee::EyeState reportedState) : reportedState_(reportedState) {
    }

    int onEnterCallCount = 0;
    int onExitCallCount = 0;
    int updateCallCount = 0;

    void onEnter(eyesee::IAnimationEngine& animation) override {
        (void)animation;
        ++onEnterCallCount;
    }
    void onExit(eyesee::IAnimationEngine& animation) override {
        (void)animation;
        ++onExitCallCount;
    }
    void update(uint32_t deltaMs, eyesee::IAnimationEngine& animation) override {
        (void)deltaMs;
        (void)animation;
        ++updateCallCount;
    }
    eyesee::EyeState state() const override {
        return reportedState_;
    }

private:
    eyesee::EyeState reportedState_;
};
