#include <unity.h>

#include "RandomBehavior.h"
#include "support/FakeAnimationEngine.h"
#include "support/FakeRandomSource.h"

using eyesee::EyeState;
using eyesee::RandomBehavior;

void test_random_behavior_waits_for_full_interval_before_glancing() {
    FakeRandomSource random;
    random.uintSequence = {400};
    random.floatSequence = {0.8f, -0.7f};
    FakeAnimationEngine animation;
    RandomBehavior behavior(random);

    behavior.onEnter(animation);
    behavior.update(200, animation);
    TEST_ASSERT_EQUAL_INT(0, animation.animateGazeCallCount);

    behavior.update(200, animation);
    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
}

void test_random_behavior_glance_uses_wider_range_and_higher_speed() {
    FakeRandomSource random;
    random.uintSequence = {100};
    random.floatSequence = {0.9f, -0.9f};
    FakeAnimationEngine animation;
    RandomBehavior behavior(random);

    behavior.onEnter(animation);
    behavior.update(150, animation);

    TEST_ASSERT_EQUAL_FLOAT(0.9f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.9f, animation.lastGazeTarget.y);
    TEST_ASSERT_EQUAL_FLOAT(600.0f, animation.lastGazeTarget.speed);
}

void test_random_behavior_reports_idle_state() {
    FakeRandomSource random;
    RandomBehavior behavior(random);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Idle), static_cast<int>(behavior.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
