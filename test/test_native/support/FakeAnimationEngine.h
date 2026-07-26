#pragma once

#include "IAnimationEngine.h"

class FakeAnimationEngine : public eyesee::IAnimationEngine {
public:
    int animateGazeCallCount = 0;
    eyesee::GazeTarget lastGazeTarget{};
    int animateBlinkCallCount = 0;
    int animateWinkLeftCallCount = 0;
    int animateWinkRightCallCount = 0;
    int animateSleepCallCount = 0;
    uint32_t lastSleepDurationMs = 0;
    int animateWakeCallCount = 0;
    int animateExpressionCallCount = 0;
    eyesee::Expression lastExpression = eyesee::Expression::Neutral;

    void animateGaze(const eyesee::GazeTarget& target) override {
        lastGazeTarget = target;
        ++animateGazeCallCount;
    }
    void animateBlink(uint32_t) override {
        ++animateBlinkCallCount;
    }
    void animateWinkLeft(uint32_t) override {
        ++animateWinkLeftCallCount;
    }
    void animateWinkRight(uint32_t) override {
        ++animateWinkRightCallCount;
    }
    void animateSleep(uint32_t durationMs) override {
        lastSleepDurationMs = durationMs;
        ++animateSleepCallCount;
    }
    void animateWake(uint32_t) override {
        ++animateWakeCallCount;
    }
    void animateExpression(eyesee::Expression expression, uint32_t) override {
        lastExpression = expression;
        ++animateExpressionCallCount;
    }
    void update(uint32_t) override {
    }
};
