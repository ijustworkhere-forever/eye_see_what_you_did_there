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

    animation.update(25);  // 25% of the way through: cubic(0.25) = 0.0625, ease-in-out(0.25) =
                           // 0.125, linear(0.25) = 0.25 — 0.10 is below both non-cubic values, so
                           // this actually pins Cubic rather than merely ruling out Linear.

    TEST_ASSERT_TRUE(controller.currentPose().lookX < 0.10f);
}

void test_eyelid_transition_uses_ease_in_out_not_cubic_or_linear() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(200);
    animation.update(50);  // 25% of the way through: ease-in-out(0.25) = 0.125, so lid = 0.875.
                           // cubic(0.25) = 0.0625 -> lid = 0.9375 (fails high); linear(0.25) =
                           // 0.25 -> lid = 0.75 (fails low). This pins EaseInOut specifically.

    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid > 0.80f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 0.90f);
}

void test_interrupting_mid_flight_transition_continues_from_current_value() {
    FakeServoOutput output;
    CalibrationManager calibration;
    EyeController controller(output, calibration);
    RealAnimationEngine animation(controller, calibration);

    animation.animateBlink(100);
    animation.update(50);  // lid now at 0.5, blink half-closed

    animation.animateWinkLeft(100);  // interrupts mid-flight; new start must be current (0.5),
                                     // not the old target (1.0, which would make the lid jump
                                     // back up before continuing down)
    animation.update(50);

    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid >= 0.0f);
    TEST_ASSERT_TRUE(controller.currentPose().upperLeftLid < 0.5f);
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
