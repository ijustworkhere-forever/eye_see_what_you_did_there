#include <unity.h>

#include "TrackingBehavior.h"
#include "support/FakeAnimationEngine.h"

using eyesee::EyeState;
using eyesee::GazeTarget;
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

    GazeTarget target;
    target.x = 0.5f;
    target.y = -0.3f;
    target.hold = true;
    behavior.receiveGazeTarget(target);
    behavior.update(16, animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(-0.3f, animation.lastGazeTarget.y);
    TEST_ASSERT_TRUE(animation.lastGazeTarget.hold);
}

void test_tracking_behavior_does_not_reissue_gaze_for_unchanged_target() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    GazeTarget target;
    target.x = 0.5f;
    target.y = -0.3f;
    target.hold = true;
    behavior.receiveGazeTarget(target);
    behavior.update(16, animation);
    behavior.update(16, animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);
}

void test_tracking_behavior_reports_tracking_state() {
    TrackingBehavior behavior;
    TEST_ASSERT_EQUAL_INT(static_cast<int>(EyeState::Tracking), static_cast<int>(behavior.state()));
}

void test_tracking_behavior_recenters_after_stale_timeout_without_hold() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    GazeTarget target;
    target.x = 0.5f;
    target.y = 0.5f;
    target.hold = false;
    behavior.receiveGazeTarget(target);
    behavior.update(16, animation);  // consumes the targetChanged_ animateGaze call
    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);

    behavior.update(3000, animation);  // exceeds kStaleTimeoutMs in one tick

    TEST_ASSERT_EQUAL_INT(2, animation.animateGazeCallCount);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, animation.lastGazeTarget.x);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, animation.lastGazeTarget.y);
}

void test_tracking_behavior_holds_indefinitely_when_hold_true() {
    FakeAnimationEngine animation;
    TrackingBehavior behavior;

    GazeTarget target;
    target.x = 0.5f;
    target.y = 0.5f;
    target.hold = true;
    behavior.receiveGazeTarget(target);
    behavior.update(16, animation);
    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);

    behavior.update(3000, animation);
    behavior.update(10000, animation);

    TEST_ASSERT_EQUAL_INT(1, animation.animateGazeCallCount);  // no recenter call ever issued
}

// No main() here — test/test_native/test_main.cpp is the sole file with main().
