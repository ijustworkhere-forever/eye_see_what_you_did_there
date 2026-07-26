#include <unity.h>

#include "IdleBehavior.h"
#include "support/FakeAnimationEngine.h"
#include "support/FakeRandomSource.h"

using eyesee::EyeState;
using eyesee::IdleBehavior;

void test_idle_behavior_waits_for_full_interval_before_glancing() {
    FakeRandomSource random;
    random.uintSequence = {1000};
    random.floatSequence = {0.1f, -0.05f};
    FakeAnimationEngine animation;
    IdleBehavior behavior(random);

    behavior.onEnter(animation);
    behavior.update(500, animation);
    TEST_ASSERT_EQUAL_INT(0, animation.animateGazeCallCount);

    behavior.update(500, animation);
    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
}

void test_idle_behavior_glance_stays_within_micro_saccade_range() {
    FakeRandomSource random;
    random.uintSequence = {100};
    random.floatSequence = {0.15f, -0.15f};
    FakeAnimationEngine animation;
    IdleBehavior behavior(random);

    behavior.onEnter(animation);
    behavior.update(200, animation);

    TEST_ASSERT_EQUAL_FLOAT(0.15f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.15f, animation.lastGazeTarget.y);
    TEST_ASSERT_EQUAL_FLOAT(300.0f, animation.lastGazeTarget.speed);
}

void test_idle_behavior_reports_idle_state() {
    FakeRandomSource random;
    IdleBehavior behavior(random);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Idle), static_cast<int>(behavior.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
