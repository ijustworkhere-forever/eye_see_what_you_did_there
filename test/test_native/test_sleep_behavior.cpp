#include <unity.h>

#include "SleepBehavior.h"
#include "support/FakeAnimationEngine.h"

using eyesee::EyeState;
using eyesee::SleepBehavior;

void test_sleep_behavior_update_does_nothing() {
    FakeAnimationEngine animation;
    SleepBehavior behavior;

    behavior.update(1000, animation);

    TEST_ASSERT_EQUAL_INT(0, animation.animateGazeCallCount);
    TEST_ASSERT_EQUAL_INT(0, animation.animateSleepCallCount);
}

void test_sleep_behavior_reports_sleeping_state() {
    SleepBehavior behavior;
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Sleeping), static_cast<int>(behavior.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
