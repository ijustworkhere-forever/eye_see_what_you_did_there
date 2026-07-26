#include <unity.h>

#include "CalibrationManager.h"
#include "EyeController.h"
#include "RealAnimationEngine.h"
#include "support/FakeServoOutput.h"

using eyesee::CalibrationManager;
using eyesee::EyeController;
using eyesee::GazeTarget;
using eyesee::RealAnimationEngine;

void test_animate_gaze_does_not_reach_target_before_duration_elapses() {
    FakeServoOutput output;
    CalibrationManager calibration;  // lookRangeDegrees defaults to 30.0
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // duration = (1.0 * 30 / 300) * 1000 = 100ms
    animation.animateGaze(target);

    animation.update(50);  // halfway through the 100ms transition

    TEST_ASSERT_TRUE(controller.currentPose().lookX > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().lookX < 1.0f);
}

void test_animate_gaze_reaches_target_after_full_duration() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms duration
    animation.animateGaze(target);

    animation.update(60);
    animation.update(60);  // 120ms total, past the 100ms duration

    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
}

void test_animate_gaze_uses_cubic_easing_not_linear() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms duration
    animation.animateGaze(target);

    animation.update(25);  // 25% of the way through — cubic ease-in-out starts slow

    TEST_ASSERT_TRUE(controller.currentPose().lookX < 0.20f);
}

void test_animate_blink_closes_lids_only_after_full_duration() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(100);
    animation.update(50);

    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.0f);

    animation.update(60);  // 110ms total, past the 100ms duration

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

void test_gaze_and_eyelid_transitions_compose_in_the_same_frame() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms
    animation.animateGaze(target);
    animation.animateBlink(200);

    animation.update(50);

    TEST_ASSERT_TRUE(controller.currentPose().lookX > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().lookX < 1.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 1.0f);
}

void test_wink_left_preserves_right_eyelids() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateWinkLeft(100);
    animation.update(100);

    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperRightLid);
}

void test_blink_on_arrival_triggers_blink_after_gaze_completes() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    GazeTarget target;
    target.x = 1.0f;
    target.speed = 300.0f;  // 100ms
    target.blinkOnArrival = true;
    animation.animateGaze(target);

    animation.update(100);  // completes gaze, should trigger a 150ms blink
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().lookX);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, controller.currentPose().upperLeftLid);  // blink not yet advanced

    animation.update(150);  // advance the triggered blink to completion
    TEST_ASSERT_EQUAL_FLOAT(0.0f, controller.currentPose().upperLeftLid);
}

// No main() here — test/test_native/test_main.cpp is the sole file with
// main() (see its own comment for why).
