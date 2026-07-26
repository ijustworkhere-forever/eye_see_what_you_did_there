#include <unity.h>

#include "TrackingBehavior.h"
#include "support/FakeAnimationEngine.h"

using eyesee::EyeState;
using eyesee::TrackingBehavior;

void test_tracking_behavior_does_nothing_until_target_is_set() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    behavior.update(16, animation);

    TEST_ASSERT_EQUAL_INT(0, animation.animateGazeCallCount);
}

void test_tracking_behavior_animates_toward_target_once_set() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    behavior.setTrackedTarget(0.5f, -0.3f);
    behavior.update(16, animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, animation.lastGazeTarget.y);
    TEST_ASSERT_TRUE(animation.lastGazeTarget.hold);
}

void test_tracking_behavior_does_not_reissue_gaze_for_unchanged_target() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    behavior.setTrackedTarget(0.5f, -0.3f);
    behavior.update(16, animation);
    behavior.update(16, animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
}

void test_tracking_behavior_reports_tracking_state() {
    TrackingBehavior behavior;
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Tracking), static_cast<int>(behavior.state()));
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
